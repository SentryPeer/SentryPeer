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
use crate::sip::{SIP_PACKET, log_sip_packet};
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::net::UdpSocket;

pub async fn handle_udp_connection(
    peer_addr: SocketAddr,
    buf: &mut [u8],
    bytes_read: usize,
    udp_socket: Arc<UdpSocket>,
    sentrypeer_config: SentryPeerConfig,
    addr: SocketAddr,
) -> Result<(), Box<dyn std::error::Error>> {
    let debug_mode = (unsafe { *sentrypeer_config.p }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_config.p }).verbose_mode;
    let sip_responsive_mode = (unsafe { *sentrypeer_config.p }).sip_responsive_mode;

    if debug_mode || verbose_mode {
        eprintln!("Received UDP packet from: {peer_addr}");
    }

    if log_sip_packet(
        sentrypeer_config,
        buf.to_vec(),
        bytes_read,
        peer_addr,
        addr,
        "UDP",
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
        udp_socket.send_to(SIP_PACKET, peer_addr).await?;
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{sentrypeer_config_destroy, sentrypeer_config_new};
    use serial_test::serial;

    const TEST_SIP_REQUEST: &[u8] = b"OPTIONS sip:100@127.0.0.1:5060 SIP/2.0\r
Via: SIP/2.0/UDP 127.0.0.1:5060;branch=z9hG4bK-5678\r
From: <sip:test@127.0.0.1>;tag=5678\r
To: <sip:100@127.0.0.1>\r
Call-ID: test-udp-call-id-5678@127.0.0.1\r
CSeq: 1 OPTIONS\r
Contact: <sip:test@127.0.0.1:5060>\r
User-Agent: test-udp-agent\r
Content-Length: 0\r\n\r\n";

    #[tokio::test]
    #[serial]
    async fn test_handle_udp_connection_responsive() {
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

        let server_socket = match UdpSocket::bind("127.0.0.1:0").await {
            Ok(sock) => Arc::new(sock),
            Err(err) => panic!("Failed to bind server UDP socket: {err}"),
        };
        let server_addr = match server_socket.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local server address: {err}"),
        };

        let client_socket = match UdpSocket::bind("127.0.0.1:0").await {
            Ok(sock) => sock,
            Err(err) => panic!("Failed to bind client UDP socket: {err}"),
        };

        if let Err(err) = client_socket.send_to(TEST_SIP_REQUEST, server_addr).await {
            panic!("Client failed to send UDP packet: {err}");
        }

        let mut buf = [0; 1024];
        let (bytes_read, peer_addr) = match server_socket.recv_from(&mut buf).await {
            Ok(res) => res,
            Err(err) => panic!("Server failed to receive UDP packet: {err}"),
        };

        let res = handle_udp_connection(
            peer_addr,
            &mut buf,
            bytes_read,
            server_socket.clone(),
            sentrypeer_config,
            server_addr,
        )
        .await;
        assert!(res.is_ok());

        let mut reply_buf = [0; 1024];
        let (reply_len, _reply_peer) = match client_socket.recv_from(&mut reply_buf).await {
            Ok(res) => res,
            Err(err) => panic!("Client failed to receive UDP reply: {err}"),
        };
        assert!(reply_len > 0);
        let reply_str = String::from_utf8_lossy(&reply_buf[..reply_len]);
        assert!(reply_str.contains("SIP/2.0 200 OK"));

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }

    #[tokio::test]
    #[serial]
    async fn test_handle_udp_connection_non_responsive() {
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

        let server_socket = match UdpSocket::bind("127.0.0.1:0").await {
            Ok(sock) => Arc::new(sock),
            Err(err) => panic!("Failed to bind server UDP socket: {err}"),
        };
        let server_addr = match server_socket.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local server address: {err}"),
        };

        let client_socket = match UdpSocket::bind("127.0.0.1:0").await {
            Ok(sock) => sock,
            Err(err) => panic!("Failed to bind client UDP socket: {err}"),
        };

        if let Err(err) = client_socket.send_to(TEST_SIP_REQUEST, server_addr).await {
            panic!("Client failed to send UDP packet: {err}");
        }

        let mut buf = [0; 1024];
        let (bytes_read, peer_addr) = match server_socket.recv_from(&mut buf).await {
            Ok(res) => res,
            Err(err) => panic!("Server failed to receive UDP packet: {err}"),
        };

        let res = handle_udp_connection(
            peer_addr,
            &mut buf,
            bytes_read,
            server_socket.clone(),
            sentrypeer_config,
            server_addr,
        )
        .await;
        assert!(res.is_ok());

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }

    #[tokio::test]
    #[serial]
    async fn test_handle_udp_connection_invalid_packet() {
        let sentrypeer_c_config = unsafe { sentrypeer_config_new() };
        assert!(!sentrypeer_c_config.is_null());
        unsafe {
            (*sentrypeer_c_config).debug_mode = false;
            (*sentrypeer_c_config).verbose_mode = true;
            (*sentrypeer_c_config).sip_responsive_mode = false;
        }
        let sentrypeer_config = SentryPeerConfig {
            p: sentrypeer_c_config,
        };

        let server_socket = match UdpSocket::bind("127.0.0.1:0").await {
            Ok(sock) => Arc::new(sock),
            Err(err) => panic!("Failed to bind server UDP socket: {err}"),
        };
        let server_addr = match server_socket.local_addr() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to get local server address: {err}"),
        };

        let mut invalid_buf = b"INVALID UDP SIP".to_vec();
        let len = invalid_buf.len();
        let dummy_peer: SocketAddr = match "127.0.0.1:9999".parse() {
            Ok(addr) => addr,
            Err(err) => panic!("Failed to parse dummy peer: {err}"),
        };

        let res = handle_udp_connection(
            dummy_peer,
            &mut invalid_buf,
            len,
            server_socket,
            sentrypeer_config,
            server_addr,
        )
        .await;
        assert!(res.is_ok());

        unsafe {
            let mut conf_ptr = sentrypeer_c_config;
            sentrypeer_config_destroy(&mut conf_ptr);
        }
    }
}
