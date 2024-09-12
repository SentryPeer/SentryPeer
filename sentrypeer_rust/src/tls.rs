/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
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
use anyhow::Context;
use dotenvy::dotenv;
use libc::c_int;
use os_socketaddr::OsSocketAddr;
use pki_types::{CertificateDer, PrivateKeyDer};
use rustls_pemfile::{certs, private_key};
use serde::Deserialize;
use std::ffi::CString;
use std::fs::File;
use std::io::{self, BufReader, ErrorKind};
use std::net::ToSocketAddrs;
use std::os::raw::c_char;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use tokio::io::{split, AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio::sync::oneshot;
use tokio_rustls::{rustls, TlsAcceptor};

// Our C FFI functions
use crate::{sentrypeer_config, sip_log_event, sip_message_event_new, sip_message_event_destroy};

#[derive(Debug, Copy, Clone)]
struct SentryPeerConfig {
    p: *mut sentrypeer_config,
}
unsafe impl Send for SentryPeerConfig {}
unsafe impl Sync for SentryPeerConfig {}

#[derive(Deserialize, Debug)]
pub struct Config {
    pub cert: PathBuf,
    pub key: PathBuf,
    pub tls_listen_address: String,
}

fn config_from_env() -> Result<Config, Box<dyn std::error::Error>> {
    // These two steps will be done in our main config, but for testing we're doing it here
    // Load the .env file
    dotenv().ok();

    let config = envy::prefixed("SENTRYPEER_").from_env::<Config>().with_context(|| {
        "Please set SENTRYPEER_CERT, SENTRYPEER_KEY and SENTRYPEER_TLS_LISTEN_ADDRESS in your .env file or environment."
    })?;

    Ok(config)
}

fn load_certs(path: &Path) -> io::Result<Vec<CertificateDer<'static>>> {
    certs(&mut BufReader::new(File::open(path)?)).collect()
}

fn load_key(path: &Path) -> io::Result<PrivateKeyDer<'static>> {
    private_key(&mut BufReader::new(File::open(path)?))?.ok_or(io::Error::new(
        ErrorKind::Other,
        "no private key found".to_string(),
    ))
}

fn log_sip_packet(
    sentrypeer_c_config: SentryPeerConfig,
    buf: Vec<u8>,
    bytes_read: usize,
    peer_addr: std::net::SocketAddr,
    listen_addr: std::net::SocketAddr,
) -> i32 {
    let mut peer_addr_c: OsSocketAddr = peer_addr.into();
    let sentrypeer_c_config = sentrypeer_c_config.p;

    // To free on our side
    // https://doc.rust-lang.org/std/ffi/struct.CString.html#method.into_raw
    let packet_ptr = CString::new(String::from_utf8_lossy(&buf[..bytes_read]).to_string())
        .unwrap()
        .into_raw();
    let transport_type_ptr = CString::new("TLS").unwrap().into_raw();
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
    let _ = CString::from_raw(packet_ptr);
    let _ = CString::from_raw(transport_type_ptr);
    let _ = CString::from_raw(client_ip_addr_ptr);
    let _ = CString::from_raw(dest_ip_addr_ptr);
}

/// # Safety
///
/// Shutdown the tokio runtime.
#[no_mangle]
pub(crate) unsafe extern "C" fn shutdown_tls(sentrypeer_c_config: *const sentrypeer_config) -> i32 {
    // Assert we're not getting a null pointer
    assert!(!sentrypeer_c_config.is_null(), "sentrypeer_c_config is null.");

    // And this
    assert!(
        !(*sentrypeer_c_config).sip_tls_channel.is_null(),
        "sentrypeer_c_config.sip_tls_handle is null."
    );

    let tx = Box::from_raw((*sentrypeer_c_config).sip_tls_channel as *mut oneshot::Sender<String>);

    // Send the message to the tokio runtime to shutdown
    if tx.send(String::from("Please shutdown :-)")).is_err() {
        eprintln!("Failed to send message to tokio runtime to shutdown");
        return libc::EXIT_FAILURE;
    }

    libc::EXIT_SUCCESS
}

/// # Safety
///
/// Nothing is done with the `sentrypeer_config` pointer, it's treated read-only.
///
/// A default multi-threaded tokio runtime that listens for incoming TLS connections.
#[no_mangle]
pub(crate) extern "C" fn listen_tls(sentrypeer_c_config: *mut sentrypeer_config) -> i32 {
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
    let thread_builder = std::thread::Builder::new().name("tls_std_thread".to_string());
    let _std_thread_handle = thread_builder.spawn(move || {
        handle.block_on(async move {
            let config = config_from_env().unwrap();

            let addr = config
                .tls_listen_address
                .to_socket_addrs()
                .unwrap()
                .next()
                .ok_or_else(|| io::Error::from(io::ErrorKind::AddrNotAvailable))
                .unwrap();
            let certs = load_certs(&config.cert).unwrap();
            let key = load_key(&config.key).unwrap();

            let config = rustls::ServerConfig::builder()
                .with_no_client_auth()
                .with_single_cert(certs, key)
                .map_err(|err| io::Error::new(io::ErrorKind::InvalidInput, err))
                .unwrap();
            let acceptor = TlsAcceptor::from(Arc::new(config));

            let listener = TcpListener::bind(&addr).await.unwrap();

            let debug_mode = (unsafe { *sentrypeer_config.p }).debug_mode;
            let verbose_mode = (unsafe { *sentrypeer_config.p }).verbose_mode;
            let sip_responsive_mode = (unsafe { *sentrypeer_config.p }).sip_responsive_mode;

            if debug_mode || verbose_mode {
                eprintln!("Listening for incoming TLS connections...");
            }

            let mut buf = [0; 1024];

            tokio::spawn(async move {
                loop {
                    let (stream, peer_addr) = listener.accept().await.unwrap();
                    let acceptor = acceptor.clone();

                    if debug_mode || verbose_mode {
                        eprintln!("Accepted TLS connection from: {}", peer_addr);
                    }

                    let fut = async move {
                        let stream = acceptor.accept(stream).await.unwrap();

                        let (mut reader, mut writer) = split(stream);
                        let bytes_read = reader.read(&mut buf).await.unwrap();

                        if log_sip_packet(sentrypeer_config, buf.to_vec(), bytes_read, peer_addr, addr)
                            != libc::EXIT_SUCCESS
                        {
                            eprintln!("Failed to log SIP packet");
                        }

                        if debug_mode || verbose_mode {
                            eprintln!(
                                "Received: {:?}",
                                String::from_utf8_lossy(&buf[..bytes_read])
                            );
                        }

                        if sip_responsive_mode {
                            writer
                                .write_all(
                                    &b"SIP/2.0 200 OK\r\n
                    Via: SIP/2.0/UDP 127.0.0.1:56940\r\n
		            Call-ID: 1179563087@127.0.0.1\r\n
		            From: <sip:sipsak@127.0.0.1>;tag=464eb44f\r\n
		            To: <sip:asterisk@127.0.0.1>;tag=z9hG4bK.1c882828\r\n
		            CSeq: 1 OPTIONS\r\n
		            Accept: application/sdp, application/dialog-info+xml, application/simple-message-summary, application/xpidf+xml, application/cpim-pidf+xml, application/pidf+xml, application/pidf+xml, application/dialog-info+xml, application/simple-message-summary, message/sipfrag;version=2.0\r\n
		            Allow: OPTIONS, SUBSCRIBE, NOTIFY, PUBLISH, INVITE, ACK, BYE, CANCEL, UPDATE, PRACK, REGISTER, REFER, MESSAGE\r\n
		            Supported: 100rel, timer, replaces, norefersub\r\n
		            Accept-Encoding: text/plain\r\n
		            Accept-Language: en\r\n
		            Server: FPBX-16.0.33(18.13.0)\r\n
		            Content-Length:  0\r\n"[..],
                                )
                                .await.unwrap();
                        }

                        libc::EXIT_SUCCESS
                    };

                    // This runs the future on the tokio runtime
                    tokio::spawn(async move {
                        if fut.await != libc::EXIT_SUCCESS {
                            eprintln!("Failed to handle TLS connection");
                        }
                    });
                }
            });

            match rx.await {
                Ok(msg) => {
                    if debug_mode || verbose_mode {
                        eprintln!("Tokio received a oneshot message to shutdown: {:?}", msg);
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
        (*sentrypeer_c_config).sip_tls_channel = Box::into_raw(Box::new(tx)) as *mut libc::c_void;
    }

    libc::EXIT_SUCCESS
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{sentrypeer_config_destroy, sentrypeer_config_new};

    // https://doc.rust-lang.org/reference/attributes/testing.html#the-ignore-attribute
    #[test]
    fn test_config_from_env() {
        let config = config_from_env().unwrap();
        assert_eq!(config.cert, PathBuf::from("../tests/tools/127.0.0.1.pem"));
        assert_eq!(
            config.key,
            PathBuf::from("../tests/tools/127.0.0.1-key.pem")
        );
        assert_eq!(config.tls_listen_address, "127.0.0.1:8088");
    }

    #[test]
    #[ignore = "not yet implemented"]
    fn test_load_certs() {
        let path = Path::new("tests/certs/cert.pem");
        let certs = load_certs(path)
            .with_context(|| "Failed to load certs")
            .unwrap();
        assert_eq!(certs.len(), 1);
    }

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

            if listen_tls(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                eprintln!("Failed to listen for TLS connections");
            }

            sentrypeer_config_destroy(&mut sentrypeer_c_config);
        }
    }
}
