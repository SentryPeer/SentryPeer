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
use crate::sip::{build_sip_reply, log_sip_packet};
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
        let reply = build_sip_reply(&buf[..bytes_read]);
        udp_socket.send_to(&reply, peer_addr).await?;
    }

    Ok(())
}
