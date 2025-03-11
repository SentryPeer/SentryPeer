/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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
use crate::cli;
use crate::BadActor;
use libc::c_char;
use std::ffi::{CStr, CString};

// Our C FFI functions
use crate::{sentrypeer_config, PACKAGE_NAME, PACKAGE_VERSION};

fn bad_actor_to_json_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor: &BadActor,
) -> *mut c_char {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    // Make our JSON by hand with serde_json::json!()
    let json = serde_json::json!({
        "app_name": cli::cstr_to_string(PACKAGE_NAME),
        "app_version": cli::cstr_to_string(PACKAGE_VERSION),
        "event_timestamp": unsafe { CStr::from_ptr(bad_actor.event_timestamp).to_str().unwrap() },
        "event_uuid": unsafe { CStr::from_ptr(bad_actor.event_uuid).to_str().unwrap() },
        "created_by_node_id": unsafe { CStr::from_ptr(bad_actor.created_by_node_id).to_str().unwrap() },
        "collected_method": unsafe { CStr::from_ptr(bad_actor.collected_method).to_str().unwrap() },
        "transport_type": unsafe { CStr::from_ptr(bad_actor.transport_type).to_str().unwrap() },
        "source_ip": unsafe { CStr::from_ptr(bad_actor.source_ip).to_str().unwrap() },
        "destination_ip": unsafe { CStr::from_ptr(bad_actor.destination_ip).to_str().unwrap() },
        "called_number": unsafe { CStr::from_ptr(bad_actor.called_number).to_str().unwrap() },
        "sip_method": unsafe { CStr::from_ptr(bad_actor.method).to_str().unwrap() },
        "sip_user_agent": unsafe { CStr::from_ptr(bad_actor.user_agent).to_str().unwrap() },
        "sip_message": unsafe { CStr::from_ptr(bad_actor.sip_message).to_str().unwrap() },
    });

    if debug_mode || verbose_mode {
        eprintln!("Bad Actor JSON: {:?}", json);
    }

    // Return the JSON as a C string which must be freed
    CString::new(json.to_string()).unwrap().into_raw()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::sentrypeer_config_new;
    use serde_json::Value;

    #[test]
    fn test_bad_actor_to_json_rs() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let bad_actor = BadActor::new(
                CString::new("blah").unwrap().into_raw(),
                CString::new("127.0.0.1").unwrap().into_raw(),
                CString::new("127.0.0.1").unwrap().into_raw(),
                CString::new("1234").unwrap().into_raw(),
                CString::new("INVITE").unwrap().into_raw(),
                CString::new("UDP").unwrap().into_raw(),
                CString::new("SIPp").unwrap().into_raw(),
                CString::new("INVITE").unwrap().into_raw(),
                CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")
                    .unwrap()
                    .into_raw(),
            );

            let bad_actor_json = bad_actor_to_json_rs(sentrypeer_c_config, &bad_actor);
            let bad_actor_json_str = CStr::from_ptr(bad_actor_json).to_str().unwrap();

            // Check our JSON string has a few expected fields
            let final_json: Value = serde_json::from_str(bad_actor_json_str).unwrap();
            dbg!(&final_json["app_name"]);
            assert_eq!(final_json["app_name"], "sentrypeer");
        }
    }
}
