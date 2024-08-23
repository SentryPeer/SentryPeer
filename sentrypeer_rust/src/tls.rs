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
use std::ffi::CString;
use std::fs::File;
use std::io::{self, BufReader, ErrorKind};
use std::net::ToSocketAddrs;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use anyhow::Context;
use dotenvy::dotenv;
use libc::c_int;
use pki_types::{CertificateDer, PrivateKeyDer};
use rustls_pemfile::{certs, private_key};
use serde::Deserialize;
use tokio::io::{split, AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio_rustls::{rustls, TlsAcceptor};
use os_socketaddr::OsSocketAddr;
use crate::sockaddr;

// Our C FFI functions
use crate::{sentrypeer_config, sip_log_event, sip_message_event_destroy, sip_message_event_new};

#[derive(Deserialize, Debug)]
pub struct Config {
    pub cert: PathBuf,
    pub key: PathBuf,
    pub listen_address: String,
}

fn config_from_env() -> Result<Config, Box<dyn std::error::Error>> {
    // These two steps will be done in our main config, but for testing we're doing it here
    // Load the .env file
    dotenv().ok();

    let config = envy::prefixed("SENTRYPEER_").from_env::<Config>().with_context(|| {
        "Please set SENTRYPEER_CERT, SENTRYPEER_KEY and SENTRYPEER_LISTEN_ADDRESS in your .env file or environment."
    })?;

    Ok(config)
}

fn load_certs(path: &Path) -> io::Result<Vec<CertificateDer<'static>>> {
    certs(&mut BufReader::new(File::open(path)?)).collect()
}

fn load_key(path: &Path) -> io::Result<PrivateKeyDer<'static>> {
    Ok(private_key(&mut BufReader::new(File::open(path)?))
        .unwrap()
        .ok_or(io::Error::new(
            ErrorKind::Other,
            "no private key found".to_string(),
        ))?)
}

#[tokio::main]
pub(crate) async fn listen(
    sentrypeer_c_config: *mut sentrypeer_config,
) -> Result<Config, Box<dyn std::error::Error>> {
    // Assert we're not getting a null pointer
    assert!(!sentrypeer_c_config.is_null());

    let config = config_from_env()?;

    let addr = config
        .listen_address
        .to_socket_addrs()?
        .next()
        .ok_or_else(|| io::Error::from(io::ErrorKind::AddrNotAvailable))?;
    let certs = load_certs(&config.cert)?;
    let key = load_key(&config.key)?;

    let config = rustls::ServerConfig::builder()
        .with_no_client_auth()
        .with_single_cert(certs, key)
        .map_err(|err| io::Error::new(io::ErrorKind::InvalidInput, err))?;
    let acceptor = TlsAcceptor::from(Arc::new(config));

    let listener = TcpListener::bind(&addr).await?;

    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;
    let sip_responsive_mode = (unsafe { *sentrypeer_c_config }).sip_responsive_mode;

    if debug_mode || verbose_mode {
        println!("Listening for incoming TLS connections...");

        if sip_responsive_mode {
            println!("SIP responsive mode enabled. Will reply to SIP probes...");
        }
    }

    let mut buf = [0; 1024];
    loop {
        let (mut stream, peer_addr) = listener.accept().await?;
        let acceptor = acceptor.clone();

        if debug_mode || verbose_mode {
            println!("Accepted TLS connection from: {}", peer_addr);
        }

        let fut = async move {
            let stream = acceptor.accept(stream).await?;

            let (mut reader, mut writer) = split(stream);
            let bytes_read = reader.read(&mut buf).await?;

            let peer_addr_c: OsSocketAddr = peer_addr.into();
            
            unsafe {
                // https://doc.rust-lang.org/std/primitive.pointer.html
                let mut sip_message = sip_message_event_new(
                    // packet from stream
                    CString::new(buf.to_vec()).unwrap().into_raw(),
                    // packet length
                    bytes_read,
                    // socket (can be anything)
                    c_int::from(0),
                    // transport_type
                    CString::new("TLS").unwrap().into_raw(),
                    // client_ip_addr
                    peer_addr_c.as_mut_ptr() as *mut sockaddr,
                    // client_ip_addr_str
                    CString::new(peer_addr.to_string()).unwrap().into_raw(),
                    // client_ip_addr_len
                    peer_addr_c.len().try_into().unwrap(),
                    // dest_ip_addr_str
                    CString::new(addr.to_string()).unwrap().into_raw(),
                );

                if sip_log_event(sentrypeer_c_config, sip_message) != libc::EXIT_SUCCESS {
                    eprintln!("Failed to log SIP message event");

                    // Clean up
                    sip_message_event_destroy(&mut sip_message);
                }

                // Clean up
                sip_message_event_destroy(&mut sip_message);
            }

            if debug_mode || verbose_mode {
                println!(
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
                            .await?;
            }

            Ok(()) as io::Result<()>
        };

        // Error
        // https://tokio.rs/tokio/tutorial/spawning#send-bound
        // future cannot be sent between threads safely future created by async block is not `Send` Help: within `{async block@src/ tls. rs:180:22: 184:10}`, the trait `std::marker::Send` is not implemented for `*mut sentrypeer_c::config::sentrypeer_config`, which is required by `{async block@src/ tls. rs:180:22: 184:10}: std::marker::Send` Note: captured value is not `Send` Note: required by a bound in `tokio::spawn`
        // This runs the future on the tokio runtime
        tokio::spawn(async move {
            if let Err(err) = fut.await {
                eprintln!("{:?}", err);
            }
        });
    }
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
        assert_eq!(config.listen_address, "127.0.0.1:8088");
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
            assert_eq!((*sentrypeer_c_config).debug_mode, true);
            assert_eq!((*sentrypeer_c_config).verbose_mode, true);
            assert_eq!((*sentrypeer_c_config).sip_responsive_mode, true);

            let result = listen(sentrypeer_c_config);

            result.unwrap();

            sentrypeer_config_destroy(&mut sentrypeer_c_config);
        }
    }
}
