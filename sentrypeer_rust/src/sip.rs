/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/
use crate::sockaddr;
use libc::c_int;
use os_socketaddr::OsSocketAddr;
use socket2::{Domain, Socket, Type};
use std::ffi::CString;
use std::io;
use std::net::{SocketAddr, ToSocketAddrs};
use std::os::raw::c_char;
use std::sync::Arc;
use tokio::io::{AsyncWriteExt, WriteHalf};
use tokio::net::TcpListener;
use tokio::net::UdpSocket;
use tokio::sync::oneshot;
use tokio_rustls::{TlsAcceptor, rustls};

use crate::config::{SentryPeerConfig, create_certs, load_all_configs, load_certs, load_key};
use crate::tcp::handle_tcp_connection;
use crate::tls::handle_tls_connection;
use crate::udp::handle_udp_connection;

// Our C FFI functions
use crate::{sentrypeer_config, sip_log_event, sip_message_event_destroy, sip_message_event_new};

// SIP packet const with \r\n - \n is added in the formatting
pub const SIP_PACKET: &[u8] = b"SIP/2.0 200 OK\r
Via: SIP/2.0/UDP 127.0.0.1:5061\r
Call-ID: 1179563087@127.0.0.1\r
From: <sip:sipsak@127.0.0.1>;tag=464eb44f\r
To: <sip:asterisk@127.0.0.1>;tag=z9hG4bK.1c882828\r
CSeq: 1 OPTIONS\r
Accept: application/sdp, application/dialog-info+xml, application/simple-message-summary, application/xpidf+xml, application/cpim-pidf+xml, application/pidf+xml, application/pidf+xml, application/dialog-info+xml, application/simple-message-summary, message/sipfrag;version=2.0\r
Allow: OPTIONS, SUBSCRIBE, NOTIFY, PUBLISH, INVITE, ACK, BYE, CANCEL, UPDATE, PRACK, REGISTER, REFER, MESSAGE\r
Supported: 100rel, timer, replaces, norefersub\r
Accept-Encoding: text/plain\r
Accept-Language: en\r
Server: FPBX-16.0.33(18.13.0)\r
Content-Length:  0\r\n";

// Allow any type that implements AsyncWriteExt so we can use tokio::net::TcpStream for TCP
// and tokio_rustls::TlsStream<tokio::net::TcpStream> for TLS, e.g. WriteHalf<TlsStream<TcpStream>
pub async fn gen_sip_reply<T>(mut writer: WriteHalf<T>)
where
    T: AsyncWriteExt,
{
    writer.write_all(SIP_PACKET).await.unwrap();
}

/// # Safety
///
/// Nothing is done with the `sentrypeer_config` pointer, it's treated read-only.
///
/// A default multi-threaded tokio runtime that listens for incoming TLS connections.
#[unsafe(no_mangle)]
pub(crate) extern "C" fn run_sip_server(sentrypeer_c_config: *mut sentrypeer_config) -> i32 {
    // Assert we're not getting a null pointer
    assert!(!sentrypeer_c_config.is_null());

    let rt = tokio::runtime::Builder::new_multi_thread()
        .thread_name("tls_tokio_runtime")
        .enable_all()
        .build()
        .unwrap();
    let handle = rt.handle().clone();

    // Create a oneshot channel to send a message to tokio runtime to shutdown
    let (tx, rx) = oneshot::channel::<String>();

    let sentrypeer_config = SentryPeerConfig {
        p: sentrypeer_c_config,
    };

    // Launch our Tokio runtime from a new thread so we can exit this function
    let thread_builder = std::thread::Builder::new().name("sentrypeer_std_thread".to_string());

    let debug_mode = (unsafe { *sentrypeer_config.p }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_config.p }).verbose_mode;

    let _std_thread_handle = thread_builder.spawn(move || {
        handle.block_on(async move {
            let config = load_all_configs(sentrypeer_config).expect("Failed to load all configs");

            // TCP
            let tcp_listener = TcpListener::bind("0.0.0.0:5060")
                .await
                .expect("TCP: Failed to bind to address");
            let addr = tcp_listener.local_addr().unwrap();

            if debug_mode || verbose_mode {
                eprintln!("Listening for incoming TCP connections...");
            }

            tokio::spawn(async move {
                loop {
                    let (stream, peer_addr) = tcp_listener.accept().await.unwrap();

                    if debug_mode || verbose_mode {
                        eprintln!("Accepted TCP connection from: {peer_addr}");
                    }

                    tokio::spawn(async move {
                        match handle_tcp_connection(stream, sentrypeer_config, peer_addr, addr)
                            .await
                        {
                            Ok(()) => libc::EXIT_SUCCESS,
                            Err(err) => {
                                eprintln!("Failed to handle TCP connection: {err}");
                                libc::EXIT_FAILURE
                            }
                        }
                    });
                }
            });

            // UDP
            let addr = "0.0.0.0:5060".parse::<SocketAddr>().unwrap();

            let socket =
                Socket::new(Domain::IPV4, Type::DGRAM, None).expect("UDP: Failed to create socket");

            socket
                .set_reuse_address(true)
                .expect("UDP: Failed to set reuse address");

            socket
                .set_nonblocking(true)
                .expect("UDP: Failed to set non-blocking");

            socket
                .bind(&addr.into())
                .expect("UDP: Failed to bind to address");

            let udp_socket =
                UdpSocket::from_std(socket.into()).expect("UDP: Failed to convert to UdpSocket");
            let addr = udp_socket.local_addr().unwrap();

            if debug_mode || verbose_mode {
                eprintln!("Listening for incoming UDP connections...");
            }
            let arc_socket = Arc::new(udp_socket);

            tokio::spawn(async move {
                loop {
                    let mut buf = [0; 1024];
                    let (bytes_read, peer_addr) = arc_socket.recv_from(&mut buf).await.unwrap();
                    let socket = arc_socket.clone();

                    // https://github.com/tokio-rs/tokio/discussions/3755#discussioncomment-702928
                    // UdpSocket docs: This type does not provide a split method, because
                    // this functionality can be achieved by instead wrapping the socket
                    // in an [Arc]
                    tokio::spawn(async move {
                        match handle_udp_connection(
                            peer_addr,
                            &mut buf,
                            bytes_read,
                            socket,
                            sentrypeer_config,
                            addr,
                        )
                        .await
                        {
                            Ok(()) => libc::EXIT_SUCCESS,
                            Err(err) => {
                                eprintln!("Failed to handle UDP connection: {err}");
                                libc::EXIT_FAILURE
                            }
                        }
                    });
                }
            });

            // TLS
            let addr = config
                .tls_listen_address
                .to_socket_addrs()
                .unwrap()
                .next()
                .ok_or_else(|| io::Error::from(io::ErrorKind::AddrNotAvailable))
                .unwrap();

            // if certs don't exist, create our default ones
            if !config.cert.exists() || !config.key.exists() {
                if debug_mode || verbose_mode {
                    eprintln!(
                        "Can't find any TLS certs, so creating default cert.pem and key.pem..."
                    );
                }

                if create_certs().is_err() {
                    eprintln!("Failed to create TLS cert and key");
                    return libc::EXIT_FAILURE;
                }
            }
            let certs = load_certs(&config.cert)
                .expect("Failed to load TLS cert. Please set SENTRYPEER_CERT or use -c");

            let key = load_key(&config.key)
                .expect("Failed to load TLS key. Please set SENTRYPEER_KEY or use -k");

            rustls::crypto::aws_lc_rs::default_provider()
                .install_default()
                .expect("Can't set crypto provider to aws_lc_rs");
            let server_config = rustls::ServerConfig::builder()
                .with_no_client_auth()
                .with_single_cert(certs, key)
                .map_err(|err| io::Error::new(io::ErrorKind::InvalidInput, err))
                .unwrap();
            let tls_acceptor = TlsAcceptor::from(Arc::new(server_config));

            let tls_listener = TcpListener::bind(addr)
                .await
                .expect("TLS: Failed to bind to address");

            if debug_mode || verbose_mode {
                eprintln!("Listening for incoming TLS connections...");
            }

            tokio::spawn(async move {
                loop {
                    let (stream, peer_addr) = tls_listener.accept().await.unwrap();
                    let acceptor = tls_acceptor.clone();

                    if debug_mode || verbose_mode {
                        eprintln!("Accepted TLS connection from: {peer_addr}");
                    }

                    tokio::spawn(async move {
                        match handle_tls_connection(
                            stream,
                            acceptor,
                            sentrypeer_config,
                            peer_addr,
                            addr,
                        )
                        .await
                        {
                            Ok(()) => libc::EXIT_SUCCESS,
                            Err(err) => {
                                eprintln!("Failed to handle TLS connection: {err}");
                                libc::EXIT_FAILURE
                            }
                        }
                    });
                }
            });

            match rx.await {
                Ok(msg) => {
                    if debug_mode || verbose_mode {
                        eprintln!("Tokio received a oneshot message to shutdown: {msg:?}");
                    }
                    // https://docs.rs/tokio/latest/tokio/runtime/struct.Runtime.html#method.shutdown_background
                    rt.shutdown_background();
                    libc::EXIT_SUCCESS
                }
                Err(_) => {
                    eprintln!("Failed to receive message to shutdown.");
                    libc::EXIT_FAILURE
                }
            }
        });
    });

    // Set the pointer to the oneshot channel
    unsafe {
        (*sentrypeer_c_config).sip_channel = Box::into_raw(Box::new(tx)) as *mut libc::c_void;
    }

    libc::EXIT_SUCCESS
}

/// # Safety
///
/// Shutdown the tokio runtime.
#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn shutdown_sip(sentrypeer_c_config: *const sentrypeer_config) -> i32 {
    unsafe {
        // Assert we're not getting a null pointer
        assert!(
            !sentrypeer_c_config.is_null(),
            "sentrypeer_c_config is null."
        );

        // And this
        assert!(
            !(*sentrypeer_c_config).sip_channel.is_null(),
            "sentrypeer_c_config.sip_channel is null."
        );

        let tx = Box::from_raw((*sentrypeer_c_config).sip_channel as *mut oneshot::Sender<String>);

        // Send the message to the tokio runtime to shutdown
        if tx.send(String::from("Please shutdown :-)")).is_err() {
            eprintln!("Failed to send message to tokio runtime to shutdown");
            return libc::EXIT_FAILURE;
        }

        libc::EXIT_SUCCESS
    }
}

pub fn log_sip_packet(
    sentrypeer_c_config: SentryPeerConfig,
    buf: Vec<u8>,
    bytes_read: usize,
    peer_addr: SocketAddr,
    listen_addr: SocketAddr,
    transport_type: &str,
) -> i32 {
    let mut peer_addr_c: OsSocketAddr = peer_addr.into();
    let sentrypeer_c_config = sentrypeer_c_config.p;

    // To free on our side
    // https://doc.rust-lang.org/std/ffi/struct.CString.html#method.into_raw
    let packet_ptr = CString::new(String::from_utf8_lossy(&buf[..bytes_read]).to_string())
        .unwrap()
        .into_raw();
    let transport_type_ptr = CString::new(transport_type).unwrap().into_raw();
    let client_ip_addr_ptr = CString::new(peer_addr.to_string()).unwrap().into_raw();
    let dest_ip_addr_ptr = CString::new(listen_addr.to_string()).unwrap().into_raw();

    unsafe {
        // https://doc.rust-lang.org/std/primitive.pointer.html
        let mut sip_message = sip_message_event_new(
            // packet from stream
            packet_ptr,
            // packet length
            bytes_read,
            // socket (can be anything)
            c_int::from(0),
            // transport_type
            transport_type_ptr,
            // client_ip_addr
            peer_addr_c.as_mut_ptr() as *mut sockaddr,
            // client_ip_addr_str
            client_ip_addr_ptr,
            // client_ip_addr_len
            peer_addr_c.len().try_into().unwrap(),
            // dest_ip_addr_str
            dest_ip_addr_ptr,
        );

        if sip_log_event(sentrypeer_c_config, sip_message) != libc::EXIT_SUCCESS {
            eprintln!("Failed to log SIP message event");

            // Clean up
            clean_up_sip_message(
                packet_ptr,
                transport_type_ptr,
                client_ip_addr_ptr,
                dest_ip_addr_ptr,
            );

            // Since we're managing the memory on the Rust side for the parts we'd
            // normally free on the C side, we need to set these pointers to null.
            // We only `free` in `sip_message_event_destroy` if they are not null.
            // Alternatively, we could just use `util_duplicate_string` and create
            // CStr on the Rust side.
            (*sip_message).packet = std::ptr::null_mut();
            (*sip_message).transport_type = std::ptr::null_mut();
            (*sip_message).client_ip_addr_str = std::ptr::null_mut();
            (*sip_message).dest_ip_addr_str = std::ptr::null_mut();
            sip_message_event_destroy(&mut sip_message);

            return libc::EXIT_FAILURE;
        }

        // Clean up
        clean_up_sip_message(
            packet_ptr,
            transport_type_ptr,
            client_ip_addr_ptr,
            dest_ip_addr_ptr,
        );
        // Since we're managing the memory on the Rust side for the parts we'd
        // normally free on the C side, we need to set these pointers to null.
        // We only `free` in `sip_message_event_destroy` if they are not null.
        // Alternatively, we could just use `util_duplicate_string` and create
        // CStr on the Rust side.
        (*sip_message).packet = std::ptr::null_mut();
        (*sip_message).transport_type = std::ptr::null_mut();
        (*sip_message).client_ip_addr_str = std::ptr::null_mut();
        (*sip_message).dest_ip_addr_str = std::ptr::null_mut();
        sip_message_event_destroy(&mut sip_message);

        libc::EXIT_SUCCESS
    }
}

unsafe fn clean_up_sip_message(
    packet_ptr: *mut c_char,
    transport_type_ptr: *mut c_char,
    client_ip_addr_ptr: *mut c_char,
    dest_ip_addr_ptr: *mut c_char,
) {
    unsafe {
        let _ = CString::from_raw(packet_ptr);
        let _ = CString::from_raw(transport_type_ptr);
        let _ = CString::from_raw(client_ip_addr_ptr);
        let _ = CString::from_raw(dest_ip_addr_ptr);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{sentrypeer_config_destroy, sentrypeer_config_new};

    #[test]
    fn test_listen() {
        unsafe {
            let mut sentrypeer_c_config = sentrypeer_config_new();
            (*sentrypeer_c_config).debug_mode = true;
            (*sentrypeer_c_config).verbose_mode = true;
            (*sentrypeer_c_config).sip_responsive_mode = true;

            assert_ne!(sentrypeer_c_config, std::ptr::null_mut());
            assert!((*sentrypeer_c_config).debug_mode);
            assert!((*sentrypeer_c_config).verbose_mode);
            assert!((*sentrypeer_c_config).sip_responsive_mode);

            if run_sip_server(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                eprintln!("Failed to listen for TLS connections");
            }

            sentrypeer_config_destroy(&mut sentrypeer_c_config);
        }
    }
}
