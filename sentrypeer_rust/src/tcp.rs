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
use crate::config::{config_from_cli, config_from_env, load_certs, load_key, SentryPeerConfig};
use crate::sip::log_sip_packet;
use crate::config;
use std::io;
use std::net::ToSocketAddrs;
use std::sync::Arc;
use tokio::io::{split, AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio::sync::oneshot;
use tokio_rustls::{rustls, TlsAcceptor};

// Our C FFI functions
use crate::sentrypeer_config;

/// # Safety
///
/// Nothing is done with the `sentrypeer_config` pointer, it's treated read-only.
///
/// A default multi-threaded tokio runtime that listens for incoming TLS connections.
#[no_mangle]
pub(crate) extern "C" fn listen_tcp(sentrypeer_c_config: *mut sentrypeer_config) -> i32 {
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

    let debug_mode = (unsafe { *sentrypeer_config.p }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_config.p }).verbose_mode;
    let sip_responsive_mode = (unsafe { *sentrypeer_config.p }).sip_responsive_mode;

    let _std_thread_handle = thread_builder.spawn(move || {
        handle.block_on(async move {
            // TODO: Move to a config function in config.rs
            // Our Configuration file is loaded first, with defaults
            let mut config = config::load_file(debug_mode, verbose_mode).expect("Failed to load config file");            
            // Then our env
            config = config_from_env(config).unwrap();
            // Then our CLI args
            config = config_from_cli(config, sentrypeer_config.p).unwrap();
            let addr = config
                .tls_listen_address
                .to_socket_addrs()
                .unwrap()
                .next()
                .ok_or_else(|| io::Error::from(io::ErrorKind::AddrNotAvailable))
                .unwrap();
            let certs = load_certs(&config.cert).expect("Failed to load TLS cert. Please set SENTRYPEER_CERT or use -t");
            let key = load_key(&config.key).expect("Failed to load TLS key. Please set SENTRYPEER_KEY or use -k");

            let config = rustls::ServerConfig::builder()
                .with_no_client_auth()
                .with_single_cert(certs, key)
                .map_err(|err| io::Error::new(io::ErrorKind::InvalidInput, err))
                .unwrap();
            let acceptor = TlsAcceptor::from(Arc::new(config));

            let listener = TcpListener::bind(&addr).await.expect("Failed to bind to address");

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

            if listen_tcp(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                eprintln!("Failed to listen for TLS connections");
            }

            sentrypeer_config_destroy(&mut sentrypeer_c_config);
        }
    }
}
