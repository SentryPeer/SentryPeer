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
use crate::config::SentryPeerConfig;
use crate::sockaddr;
use libc::c_int;
use os_socketaddr::OsSocketAddr;
use std::ffi::CString;
use std::os::raw::c_char;

// Our C FFI functions
use crate::{sip_log_event, sip_message_event_destroy, sip_message_event_new};

pub fn log_sip_packet(
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

#[cfg(test)]
mod tests {
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

            if listen_tls(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                eprintln!("Failed to listen for TLS connections");
            }

            sentrypeer_config_destroy(&mut sentrypeer_c_config);
        }
    }
}
