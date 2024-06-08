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

use chrono::Utc;
use libc::c_char;
use std::ffi::CString;
use uuid::Uuid;

//pub mod tls;

/// A manually created struct to represent a BadActor from bad_actor.h
#[repr(C)]
pub struct BadActor {
    pub event_timestamp: *const c_char,
    pub event_uuid: *const c_char,
    pub collected_method: *const c_char,
    pub created_by_node_id: *const c_char,
    pub sip_message: *const c_char,
    pub source_ip: *const c_char,
    pub destination_ip: *const c_char,
    pub called_number: *const c_char,
    pub method: *const c_char,
    pub transport_type: *const c_char,
    pub user_agent: *const c_char,
    pub seen_last: *const c_char,
    pub seen_count: *const c_char,
}

/// An associated function called new() for the BadActor struct
impl BadActor {
    pub fn new(
        sip_message: *const c_char,
        source_ip: *const c_char,
        destination_ip: *const c_char,
        called_number: *const c_char,
        method: *const c_char,
        transport_type: *const c_char,
        user_agent: *const c_char,
        collected_method: *const c_char,
        created_by_node_id: *const c_char,
    ) -> Self {
        BadActor {
            event_timestamp: CString::new(Utc::now().format("%Y-%m-%d %H:%M:%S").to_string())
                .unwrap()
                .into_raw(),
            event_uuid: CString::new(Uuid::new_v4().to_string()).unwrap().into_raw(),
            sip_message,
            source_ip,
            destination_ip,
            called_number,
            method,
            transport_type,
            user_agent,
            collected_method,
            created_by_node_id,
            seen_last: std::ptr::null(),
            seen_count: std::ptr::null(),
        }
    }
}

/// Initialize a BadActor struct and return a pointer to it
#[no_mangle]
pub extern "C" fn return_bad_actor_new(
    sip_message: *const c_char,
    source_ip: *const c_char,
    destination_ip: *const c_char,
    called_number: *const c_char,
    method: *const c_char,
    transport_type: *const c_char,
    user_agent: *const c_char,
    collected_method: *const c_char,
    created_by_node_id: *const c_char,
) -> *mut BadActor {
    Box::into_raw(Box::new(BadActor::new(
        sip_message,
        source_ip,
        destination_ip,
        called_number,
        method,
        transport_type,
        user_agent,
        collected_method,
        created_by_node_id,
    )))
}

/// # Safety
///
/// This function is unsafe because it dereferences a raw pointer for
/// the whole BadActor struct and its CString fields (from_raw).
///
/// Destroy a BadActor struct
#[no_mangle]
pub unsafe extern "C" fn bad_actor_free(bad_actor: *mut BadActor) {
    if !bad_actor.is_null() {
        unsafe {
            // Free each of the strings in the struct
            if !(*bad_actor).event_timestamp.is_null() {
                let _ = CString::from_raw((*bad_actor).event_timestamp as *mut c_char);
            }

            if !(*bad_actor).event_uuid.is_null() {
                let _ = CString::from_raw((*bad_actor).event_uuid as *mut c_char);
            }

            if !(*bad_actor).sip_message.is_null() {
                let _ = CString::from_raw((*bad_actor).sip_message as *mut c_char);
            }

            if !(*bad_actor).source_ip.is_null() {
                let _ = CString::from_raw((*bad_actor).source_ip as *mut c_char);
            }

            if !(*bad_actor).destination_ip.is_null() {
                let _ = CString::from_raw((*bad_actor).destination_ip as *mut c_char);
            }

            if !(*bad_actor).called_number.is_null() {
                let _ = CString::from_raw((*bad_actor).called_number as *mut c_char);
            }

            if !(*bad_actor).method.is_null() {
                let _ = CString::from_raw((*bad_actor).method as *mut c_char);
            }

            if !(*bad_actor).transport_type.is_null() {
                let _ = CString::from_raw((*bad_actor).transport_type as *mut c_char);
            }

            if !(*bad_actor).user_agent.is_null() {
                let _ = CString::from_raw((*bad_actor).user_agent as *mut c_char);
            }

            if !(*bad_actor).collected_method.is_null() {
                let _ = CString::from_raw((*bad_actor).collected_method as *mut c_char);
            }

            if !(*bad_actor).created_by_node_id.is_null() {
                let _ = CString::from_raw((*bad_actor).created_by_node_id as *mut c_char);
            }

            if !(*bad_actor).seen_last.is_null() {
                let _ = CString::from_raw((*bad_actor).seen_last as *mut c_char);
            }

            if !(*bad_actor).seen_count.is_null() {
                let _ = CString::from_raw((*bad_actor).seen_count as *mut c_char);
            }

            // Free the struct itself
            let _ = Box::from_raw(bad_actor);
        }
    }
}

/// The simplest function used to confirm that calling our Rust library from C is working
#[no_mangle]
pub extern "C" fn display_rust() {
    println!("Greetings from Rust");
}

/// Return libc::EXIT_SUCCESS or libc::EXIT_FAILURE depending on the function argument
#[no_mangle]
pub extern "C" fn return_exit_status(success: bool) -> i32 {
    if success {
        libc::EXIT_SUCCESS
    } else {
        libc::EXIT_FAILURE
    }
}

/// # Safety
///
/// Return a string
///
/// The caller is responsible for freeing the string. Generally, the caller
/// from the C FFI side.
#[no_mangle]
pub extern "C" fn return_string() -> *mut c_char {
    let string = CString::new("Greetings from Rust").unwrap();
    string.into_raw()
}

/// # Safety
///
/// Free the string allocated by into_raw from return_string
#[no_mangle]
pub unsafe extern "C" fn free_string(ptr_s: *mut c_char) {
    unsafe {
        if ptr_s.is_null() {
            return;
        }
        let _ = CString::from_raw(ptr_s);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CStr;

    #[test]
    fn it_works() {
        display_rust();
    }

    #[test]
    fn test_return_exit_status() {
        assert_eq!(return_exit_status(true), libc::EXIT_SUCCESS);
        assert_eq!(return_exit_status(false), libc::EXIT_FAILURE);
    }

    #[test]
    fn test_return_string() {
        let string = unsafe { CStr::from_ptr(return_string()).to_str().unwrap() };
        assert_eq!(string, "Greetings from Rust");
    }

    #[test]
    fn test_free_string() {
        let ptr = return_string();
        unsafe {
            free_string(ptr);
        }
    }

    #[test]
    fn test_return_bad_actor_new() {
        let sip_message = CString::new("blah").unwrap().into_raw();
        let source_ip = CString::new("127.0.0.1").unwrap().into_raw();
        let destination_ip = CString::new("127.0.0.1").unwrap().into_raw();
        let called_number = CString::new("1234").unwrap().into_raw();
        let method = CString::new("INVITE").unwrap().into_raw();
        let transport_type = CString::new("UDP").unwrap().into_raw();
        let user_agent = CString::new("SIPp").unwrap().into_raw();
        let collected_method = CString::new("INVITE").unwrap().into_raw();
        // Fake UUID
        let created_by_node_id = CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")
            .unwrap()
            .into_raw();

        let bad_actor = return_bad_actor_new(
            sip_message,
            source_ip,
            destination_ip,
            called_number,
            method,
            transport_type,
            user_agent,
            collected_method,
            created_by_node_id,
        );

        unsafe {
            let bad_actor = Box::from_raw(bad_actor);

            assert_eq!(
                CStr::from_ptr(bad_actor.event_timestamp).to_str().unwrap(),
                Utc::now().format("%Y-%m-%d %H:%M:%S").to_string()
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.event_uuid).to_str().unwrap().len(),
                36
            ); // We can't check the exact UUID, but we can check the length
            assert_eq!(
                CStr::from_ptr(bad_actor.sip_message).to_str().unwrap(),
                "blah"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.source_ip).to_str().unwrap(),
                "127.0.0.1"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.destination_ip).to_str().unwrap(),
                "127.0.0.1"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.called_number).to_str().unwrap(),
                "1234"
            );
            assert_eq!(CStr::from_ptr(bad_actor.method).to_str().unwrap(), "INVITE");
            assert_eq!(
                CStr::from_ptr(bad_actor.transport_type).to_str().unwrap(),
                "UDP"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.user_agent).to_str().unwrap(),
                "SIPp"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.collected_method).to_str().unwrap(),
                "INVITE"
            );
            assert_eq!(
                CStr::from_ptr(bad_actor.created_by_node_id)
                    .to_str()
                    .unwrap(),
                "460f30e4-ce1d-4d53-9004-dd40a1c4abc9"
            );

            bad_actor_free(Box::into_raw(bad_actor));
        }
    }

    // #[test]
    // fn test_tls_listen() {
    //     tls::listen().unwrap();
    // }
}
