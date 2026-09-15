/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2026 Gavin Henry <ghenry@sentrypeer.org> */
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
use uuid::Uuid;

// Our C FFI functions
use crate::{sentrypeer_config, sip_log_event, sip_message_event_destroy, sip_message_event_new};

/// Pulls a single header's value out of a raw SIP request. Case-insensitive
/// on the header name, per RFC 3261. Works line-by-line on a lossily
/// decoded copy of the request so it can never panic on non-UTF-8 or
/// embedded-NUL bytes (a peer can and does send both).
fn extract_header_value(request: &str, header_name: &str) -> Option<String> {
    let header_name_lower = header_name.to_lowercase();
    for line in request.split("\r\n") {
        let (name, value) = line.split_once(':')?;
        if name.trim().to_lowercase() == header_name_lower {
            return Some(value.trim().to_string());
        }
    }
    None
}

/// Builds a SIP 200 OK reply that correlates to the given request's
/// Via/From/To/Call-ID/CSeq, per RFC 3261 SS8.2.6, instead of always
/// replying with the same static packet regardless of what was sent to us.
/// Bad Actors doing simple scanning don't care, but anything that checks
/// whether our response actually correlates to its own request (real SIP
/// clients doing loop detection, and some OSINT/recon scanners) would
/// notice Via/From/To/Call-ID/CSeq never changed.
///
/// Falls back to the previous static values for any header the request
/// didn't include, so a malformed or truncated request still gets a
/// plausible reply instead of an empty one.
pub fn build_sip_reply(request: &[u8]) -> Vec<u8> {
    let request_str = String::from_utf8_lossy(request);

    let via = extract_header_value(&request_str, "Via")
        .unwrap_or_else(|| "SIP/2.0/UDP 127.0.0.1:56940".to_string());
    let from = extract_header_value(&request_str, "From")
        .unwrap_or_else(|| "<sip:sipsak@127.0.0.1>;tag=464eb44f".to_string());
    let mut to = extract_header_value(&request_str, "To")
        .unwrap_or_else(|| "<sip:asterisk@127.0.0.1>".to_string());
    let call_id = extract_header_value(&request_str, "Call-ID")
        .unwrap_or_else(|| "1179563087@127.0.0.1".to_string());
    let cseq = extract_header_value(&request_str, "CSeq")
        .unwrap_or_else(|| "1 OPTIONS".to_string());

    // A final response needs a To-tag (RFC 3261 SS8.2.6.2). The request
    // won't carry one on an initial transaction, so mint one if the
    // request's To header didn't already have one (an in-dialog probe
    // would already have one, and we leave that as-is).
    if !to.to_lowercase().contains(";tag=") {
        let tag = Uuid::new_v4().to_string();
        to.push_str(&format!(";tag={}", &tag[..8]));
    }

    format!(
        "SIP/2.0 200 OK\r\n\
         Via: {via}\r\n\
         Call-ID: {call_id}\r\n\
         From: {from}\r\n\
         To: {to}\r\n\
         CSeq: {cseq}\r\n\
         Accept: application/sdp, application/dialog-info+xml, application/simple-message-summary, application/xpidf+xml, application/cpim-pidf+xml, application/pidf+xml, application/pidf+xml, application/dialog-info+xml, application/simple-message-summary, message/sipfrag;version=2.0\r\n\
         Allow: OPTIONS, SUBSCRIBE, NOTIFY, PUBLISH, INVITE, ACK, BYE, CANCEL, UPDATE, PRACK, REGISTER, REFER, MESSAGE\r\n\
         Supported: 100rel, timer, replaces, norefersub\r\n\
         Accept-Encoding: text/plain\r\n\
         Accept-Language: en\r\n\
         Server: FPBX-17.0.32(22.6.0)\r\n\
         Content-Length:  0\r\n"
    )
    .into_bytes()
}

// Allow any type that implements AsyncWriteExt so we can use tokio::net::TcpStream for TCP
// and tokio_rustls::TlsStream<tokio::net::TcpStream> for TLS, e.g. WriteHalf<TlsStream<TcpStream>
pub async fn gen_sip_reply<T>(mut writer: WriteHalf<T>, request: &[u8])
where
    T: AsyncWriteExt,
{
    writer.write_all(&build_sip_reply(request)).await.unwrap();
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

    #[test]
    fn test_build_sip_reply_correlates_request_headers() {
        let request = b"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n\
            Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-verify-test\r\n\
            From: <sip:probe@0.0.0.0>;tag=verifytest1\r\n\
            To: <sip:1000@127.0.0.1>\r\n\
            Call-ID: verify-test-call-id-12345\r\n\
            CSeq: 42 OPTIONS\r\n\
            Content-Length: 0\r\n\r\n";

        let reply = String::from_utf8(build_sip_reply(request)).unwrap();

        assert!(reply.starts_with("SIP/2.0 200 OK\r\n"));
        assert!(reply.contains(
            "Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-verify-test\r\n"
        ));
        assert!(reply.contains("From: <sip:probe@0.0.0.0>;tag=verifytest1\r\n"));
        assert!(reply.contains("Call-ID: verify-test-call-id-12345\r\n"));
        assert!(reply.contains("CSeq: 42 OPTIONS\r\n"));
        // The request's To header had no tag, so the reply must mint one
        // (RFC 3261 SS8.2.6.2) rather than echo the request verbatim.
        assert!(reply.contains("To: <sip:1000@127.0.0.1>;tag="));
        // The stale FreePBX 16/Asterisk 18 fingerprint must be gone.
        assert!(!reply.contains("FPBX-16.0.33(18.13.0)"));
        assert!(reply.contains("Server: FPBX-17.0.32(22.6.0)\r\n"));
    }

    #[test]
    fn test_build_sip_reply_preserves_existing_to_tag() {
        // As if this were an in-dialog probe - the To header already has a
        // tag, so the reply must preserve it exactly, not mint a second one.
        let request = b"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n\
            Via: SIP/2.0/UDP 0.0.0.0:1;branch=z9hG4bK-x\r\n\
            From: <sip:probe@0.0.0.0>;tag=abc\r\n\
            To: <sip:1000@127.0.0.1>;tag=already-present\r\n\
            Call-ID: has-a-to-tag-already\r\n\
            CSeq: 1 OPTIONS\r\n\
            Content-Length: 0\r\n\r\n";

        let reply = String::from_utf8(build_sip_reply(request)).unwrap();

        assert!(reply.contains("To: <sip:1000@127.0.0.1>;tag=already-present\r\n"));
        assert!(!reply.contains("tag=already-present;tag="));
    }

    #[test]
    fn test_build_sip_reply_survives_embedded_nul_byte() {
        // The same packet shape that used to panic the C string conversion
        // path (see test_packet_bytes_to_cstring_never_panics_on_embedded_nul)
        // - this must not panic here either.
        let request = b"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n\
            Via: SIP/2.0/UDP 0.0.0.0:1;branch=z9hG4bK-nul\r\n\
            From: <sip:probe@0.0.0.0>;tag=abc\r\n\
            To: <sip:1000@127.0.0.1>\r\n\
            Call-ID: has-embedded-nul\r\n\
            CSeq: 1 OPTIONS\r\n\
            Content-Length: 4\r\n\r\nA\x00BC";

        let reply = String::from_utf8(build_sip_reply(request)).unwrap();

        assert!(reply.contains("Call-ID: has-embedded-nul\r\n"));
    }

    #[test]
    fn test_build_sip_reply_falls_back_on_malformed_request() {
        let request = b"not a real SIP request at all";

        let reply = String::from_utf8(build_sip_reply(request)).unwrap();

        assert!(reply.starts_with("SIP/2.0 200 OK\r\n"));
        assert!(reply.contains("Call-ID: 1179563087@127.0.0.1\r\n"));
    }
}
