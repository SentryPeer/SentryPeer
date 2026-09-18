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
use crate::config::SentryPeerConfig;
use crate::sip::{gen_sip_reply, log_sip_packet};
use std::net::SocketAddr;
use tokio::io::{AsyncReadExt, split};
use tokio::net::TcpStream;
use tokio_rustls::TlsAcceptor;

pub async fn handle_tls_connection(
    stream: TcpStream,
    acceptor: TlsAcceptor,
    sentrypeer_config: SentryPeerConfig,
    peer_addr: SocketAddr,
    addr: SocketAddr,
) -> Result<(), Box<dyn std::error::Error>> {
    let mut buf = [0; 1024];
    let tls_stream = acceptor.accept(stream).await?;

    let (mut reader, writer) = split(tls_stream);
    let bytes_read = reader.read(&mut buf).await?;
    let debug_mode = (unsafe { *sentrypeer_config.p }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_config.p }).verbose_mode;
    let sip_responsive_mode = (unsafe { *sentrypeer_config.p }).sip_responsive_mode;

    if log_sip_packet(
        sentrypeer_config,
        buf.to_vec(),
        bytes_read,
        peer_addr,
        addr,
        "TLS",
    ) != libc::EXIT_SUCCESS
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
        gen_sip_reply(writer).await;
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{sentrypeer_config_destroy, sentrypeer_config_new};
    use pki_types::{CertificateDer, PrivateKeyDer, ServerName};
    use rcgen::generate_simple_self_signed;
    use serial_test::serial;
    use std::sync::Arc;
    use tokio::io::AsyncWriteExt;
    use tokio::net::TcpListener;
    use tokio_rustls::TlsConnector;
    use tokio_rustls::rustls::{ClientConfig, RootCertStore, ServerConfig};

    const TEST_SIP_REQUEST: &[u8] = b"OPTIONS sip:100@127.0.0.1:5061 SIP/2.0\r
Via: SIP/2.0/TLS 127.0.0.1:5061;branch=z9hG4bK-9999\r
From: <sip:test@127.0.0.1>;tag=9999\r
To: <sip:100@127.0.0.1>\r
Call-ID: test-tls-call-id-9999@127.0.0.1\r
CSeq: 1 OPTIONS\r
Contact: <sip:test@127.0.0.1:5061>\r
User-Agent: test-tls-agent\r
Content-Length: 0\r\n\r\n";

    fn setup_tls_server_and_client() -> (TlsAcceptor, TlsConnector) {
        let cert_key = match generate_simple_self_signed(vec!["localhost".to_string()]) {
            Ok(ck) => ck,
            Err(err) => panic!("Failed to generate self-signed cert: {err}"),
        };

        let cert_der = CertificateDer::from(cert_key.cert.der().to_vec());
        let key_der = match PrivateKeyDer::try_from(cert_key.signing_key.serialize_der()) {
            Ok(k) => k,
            Err(err) => panic!("Failed to serialize private key der: {err:?}"),
        };

        let server_config = match ServerConfig::builder()
            .with_no_client_auth()
            .with_single_cert(vec![cert_der.clone()], key_der)
        {
            Ok(config) => config,
            Err(err) => panic!("Failed to build server config: {err}"),
        };
        let acceptor = TlsAcceptor::from(Arc::new(server_config));

        let mut root_store = RootCertStore::empty();
        if let Err(err) = root_store.add(cert_der) {
            panic!("Failed to add cert to root store: {err}");
        }

        let client_config = ClientConfig::builder()
            .with_root_certificates(root_store)
            .with_no_client_auth();
        let connector = TlsConnector::from(Arc::new(client_config));

        (acceptor, connector)
    }

    #[tokio::test]
    #[serial]
    async fn test_handle_tls_connection_responsive() {
        let (acceptor, connector) = setup_tls_server_and_client();

        let sentrypeer_c_config = unsafe { sentrypeer_config_new() };
        assert!(!sentrypeer_c_config.is_null());
        unsafe {
            (*sentrypeer_c_config).debug_mode = true;
            (*sentrypeer_c_config).verbose_mode = true;
            (*sentrypeer_c_config).sip_responsive_mode = true;
        }
        let sentrypeer_config = SentryPeerConfig {
            p: sentrypeer_c_config,
        };

        let listener = match TcpListener::bind("127.0.0.1:0").await {
            Ok(listener) => listener,
            Err(err) => panic!("Failed to bind TCP listener: {err}"),
        };
        let listen_addr = match listener.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local address: {err}"),
        };

        let client_task = tokio::spawn(async move {
            let tcp_stream = match TcpStream::connect(listen_addr).await {
                Ok(stream) => stream,
                Err(err) => panic!("Client failed to connect: {err}"),
            };
            let server_name = match ServerName::try_from("localhost") {
                Ok(name) => name,
                Err(err) => panic!("Invalid server name: {err}"),
            };
            let mut tls_stream = match connector.connect(server_name, tcp_stream).await {
                Ok(stream) => stream,
                Err(err) => panic!("TLS handshake failed for client: {err}"),
            };

            if let Err(err) = tls_stream.write_all(TEST_SIP_REQUEST).await {
                panic!("Client failed to write SIP request over TLS: {err}");
            }

            let mut response_buf = [0; 1024];
            let n = match tls_stream.read(&mut response_buf).await {
                Ok(n) => n,
                Err(err) => panic!("Client failed to read TLS response: {err}"),
            };
            assert!(n > 0);
            let response = String::from_utf8_lossy(&response_buf[..n]);
            assert!(response.contains("SIP/2.0 200 OK"));
        });

        let (server_stream, peer_addr) = match listener.accept().await {
            Ok(res) => res,
            Err(err) => panic!("Failed to accept TCP stream: {err}"),
        };

        let res = handle_tls_connection(
            server_stream,
            acceptor,
            sentrypeer_config,
            peer_addr,
            listen_addr,
        )
        .await;
        assert!(res.is_ok());

        if let Err(err) = client_task.await {
            panic!("Client task failed: {err}");
        }

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }

    #[tokio::test]
    #[serial]
    async fn test_handle_tls_connection_non_responsive() {
        let (acceptor, connector) = setup_tls_server_and_client();

        let sentrypeer_c_config = unsafe { sentrypeer_config_new() };
        assert!(!sentrypeer_c_config.is_null());
        unsafe {
            (*sentrypeer_c_config).debug_mode = true;
            (*sentrypeer_c_config).verbose_mode = false;
            (*sentrypeer_c_config).sip_responsive_mode = false;
        }
        let sentrypeer_config = SentryPeerConfig {
            p: sentrypeer_c_config,
        };

        let listener = match TcpListener::bind("127.0.0.1:0").await {
            Ok(listener) => listener,
            Err(err) => panic!("Failed to bind TCP listener: {err}"),
        };
        let listen_addr = match listener.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local address: {err}"),
        };

        let client_task = tokio::spawn(async move {
            let tcp_stream = match TcpStream::connect(listen_addr).await {
                Ok(stream) => stream,
                Err(err) => panic!("Client failed to connect: {err}"),
            };
            let server_name = match ServerName::try_from("localhost") {
                Ok(name) => name,
                Err(err) => panic!("Invalid server name: {err}"),
            };
            let mut tls_stream = match connector.connect(server_name, tcp_stream).await {
                Ok(stream) => stream,
                Err(err) => panic!("TLS handshake failed for client: {err}"),
            };

            if let Err(err) = tls_stream.write_all(TEST_SIP_REQUEST).await {
                panic!("Client failed to write SIP request over TLS: {err}");
            }
        });

        let (server_stream, peer_addr) = match listener.accept().await {
            Ok(res) => res,
            Err(err) => panic!("Failed to accept TCP stream: {err}"),
        };

        let res = handle_tls_connection(
            server_stream,
            acceptor,
            sentrypeer_config,
            peer_addr,
            listen_addr,
        )
        .await;
        assert!(res.is_ok());

        if let Err(err) = client_task.await {
            panic!("Client task failed: {err}");
        }

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }

    #[tokio::test]
    #[serial]
    async fn test_handle_tls_connection_handshake_failure() {
        let (acceptor, _) = setup_tls_server_and_client();

        let sentrypeer_c_config = unsafe { sentrypeer_config_new() };
        assert!(!sentrypeer_c_config.is_null());
        let sentrypeer_config = SentryPeerConfig {
            p: sentrypeer_c_config,
        };

        let listener = match TcpListener::bind("127.0.0.1:0").await {
            Ok(listener) => listener,
            Err(err) => panic!("Failed to bind TCP listener: {err}"),
        };
        let listen_addr = match listener.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local address: {err}"),
        };

        let client_task = tokio::spawn(async move {
            let mut tcp_stream = match TcpStream::connect(listen_addr).await {
                Ok(stream) => stream,
                Err(err) => panic!("Client failed to connect: {err}"),
            };
            let _ = tcp_stream.write_all(b"PLAIN TCP NOT TLS\r\n").await;
        });

        let (server_stream, peer_addr) = match listener.accept().await {
            Ok(res) => res,
            Err(err) => panic!("Failed to accept TCP stream: {err}"),
        };

        let res = handle_tls_connection(
            server_stream,
            acceptor,
            sentrypeer_config,
            peer_addr,
            listen_addr,
        )
        .await;
        assert!(res.is_err());

        let _ = client_task.await;

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }
}
