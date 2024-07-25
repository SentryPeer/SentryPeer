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

use std::fs::File;
use std::io::{self, BufReader, ErrorKind};
use std::net::ToSocketAddrs;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use anyhow::Context;
use dotenvy::dotenv;
use pki_types::{CertificateDer, PrivateKeyDer};
use rustls_pemfile::{certs, private_key};
use serde::Deserialize;
use tokio::io::{copy, sink, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio_rustls::{rustls, TlsAcceptor};

// Our C FFI functions
use sentrypeer_sip_daemon::{sentrypeer_config, sip_log_event, sip_message_event, sip_send_reply};

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
    config: *mut sentrypeer_config,
) -> Result<Config, Box<dyn std::error::Error>> {
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

    loop {
        let (stream, peer_addr) = listener.accept().await?;
        let acceptor = acceptor.clone();

        let fut = async move {
            let mut stream = acceptor.accept(stream).await?;

            // How to I log this from with Rust? Call my C functions or pass in a callback?
            let mut output = sink(); // What is this?
            stream
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
            stream.shutdown().await?;
            copy(&mut stream, &mut output).await?;
            println!("Hello: {}", peer_addr);

            Ok(()) as io::Result<()>
        };

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

    // https://doc.rust-lang.org/reference/attributes/testing.html#the-ignore-attribute
    #[test]
    fn test_config_from_env() {
        let config = config_from_env().unwrap();
        assert_eq!(config.cert, PathBuf::from("./cert.pem"));
        assert_eq!(config.key, PathBuf::from("./key.pem"));
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
    #[ignore = "not yet implemented"]
    fn test_listen() {
        let result = listen();
        assert!(result.is_ok());
    }
}
