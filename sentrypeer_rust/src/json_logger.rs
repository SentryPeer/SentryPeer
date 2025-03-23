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
use libc::c_char;
use std::ffi::{CStr, CString};
use std::fs::OpenOptions;
use std::io::{BufWriter, Write};
// Our C FFI functions
use crate::{bad_actor, bad_actor_new, sentrypeer_config, PACKAGE_NAME, PACKAGE_VERSION};

#[no_mangle]
pub(crate) unsafe extern "C" fn bad_actor_to_json_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> *mut c_char {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    // Make our JSON by hand with serde_json::json!()
    let json = serde_json::json!({
        "app_name": cli::cstr_to_string(PACKAGE_NAME),
        "app_version": cli::cstr_to_string(PACKAGE_VERSION),
        "event_timestamp":
        if (*bad_actor_event).event_timestamp.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).event_timestamp).to_str().unwrap() }
        },
        "event_uuid":
        if (*bad_actor_event).event_uuid.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).event_uuid).to_str().unwrap() }
        },
        "created_by_node_id":
        if (*bad_actor_event).created_by_node_id.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).created_by_node_id).to_str().unwrap() }
        },
        "collected_method":
        if (*bad_actor_event).collected_method.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).collected_method).to_str().unwrap() }
        },
        "transport_type":
        if (*bad_actor_event).transport_type.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).transport_type).to_str().unwrap() }
        },
        "source_ip":
        if (*bad_actor_event).source_ip.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).source_ip).to_str().unwrap() }
        },
        "destination_ip":
        if (*bad_actor_event).destination_ip.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).destination_ip).to_str().unwrap() }
        },
        "called_number":
        if (*bad_actor_event).called_number.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).called_number).to_str().unwrap() }
        },
        "sip_method":
        if (*bad_actor_event).method.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).method).to_str().unwrap() }
        },
        "sip_user_agent":
        if (*bad_actor_event).user_agent.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).user_agent).to_str().unwrap() }
        },
        "sip_message":
        if (*bad_actor_event).sip_message.is_null() {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).sip_message).to_str().unwrap() }
        },
    });

    if debug_mode || verbose_mode {
        eprintln!("Bad actor in JSON format: {:?}", json);
    }

    // Return the JSON as a C string which must be freed
    CString::new(json.to_string()).unwrap().into_raw()
}

#[no_mangle]
pub(crate) unsafe extern "C" fn free_json_rs(json: *mut c_char) {
    if json.is_null() {
        return;
    }

    let _ = CString::from_raw(json);
}

#[no_mangle]
pub(crate) unsafe extern "C" fn json_to_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    json_to_convert: *const c_char,
) -> *mut bad_actor {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    let json_str = unsafe { CStr::from_ptr(json_to_convert).to_str().unwrap() };
    if debug_mode || verbose_mode {
        eprintln!("JSON to convert to a bad_actor: {:?}", json_str);
    }

    let v: serde_json::Value = serde_json::from_str(json_str).unwrap();

    let bad_actor_event = bad_actor_new(
        CString::new(v["sip_message"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["source_ip"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["destination_ip"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["called_number"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["sip_method"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["transport_type"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["sip_user_agent"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["collected_method"].as_str().unwrap())
            .unwrap()
            .into_raw(),
        CString::new(v["created_by_node_id"].as_str().unwrap())
            .unwrap()
            .into_raw(),
    );

    bad_actor_event
}

#[no_mangle]
pub(crate) unsafe extern "C" fn json_log_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> i32 {
    let log_file_name = unsafe {
        CStr::from_ptr((*sentrypeer_c_config).json_log_file)
            .to_str()
            .unwrap()
    };

    let json = bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event);
    let json_str = unsafe { CStr::from_ptr(json).to_str().unwrap() };

    let json_log_file = match OpenOptions::new()
        .append(true)
        .create(true)
        .open(log_file_name)
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Could not open JSON log file: {}", e);
            return libc::EXIT_FAILURE;
        }
    };

    let mut buf = BufWriter::new(json_log_file);
    let json_str = format!("{}\n", json_str);

    match buf.write_all(json_str.as_bytes()) {
        Ok(_) => (),
        Err(e) => {
            eprintln!("Error writing to JSON log file: {}", e);
            return libc::EXIT_FAILURE;
        }
    }

    libc::EXIT_SUCCESS
}

#[no_mangle]
pub(crate) unsafe extern "C" fn json_http_post_bad_actor_rs(
    sentrypeer_c_config: *mut sentrypeer_config,
    bad_actor_event: *const bad_actor,
) {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    let json = bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event);
    let json_str = unsafe { CStr::from_ptr(json).to_str().unwrap() };

    if debug_mode || verbose_mode {
        eprintln!("Bad actor in JSON format: {:?}", json_str);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{bad_actor_destroy, bad_actor_new, sentrypeer_config_new, util_duplicate_string};
    use pretty_assertions::assert_str_eq;
    use serde_json::Value;

    #[test]
    fn test_bad_actor_to_json_rs() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let bad_actor_event = bad_actor_new(
                util_duplicate_string(CString::new("SIP Message").unwrap().as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1").unwrap().as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1").unwrap().as_ptr()),
                util_duplicate_string(CString::new("441234512346").unwrap().as_ptr()),
                util_duplicate_string(CString::new("INVITE").unwrap().as_ptr()),
                util_duplicate_string(CString::new("TLS").unwrap().as_ptr()),
                util_duplicate_string(CString::new("SIPp").unwrap().as_ptr()),
                util_duplicate_string(CString::new("responsive").unwrap().as_ptr()),
                util_duplicate_string(
                    CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")
                        .unwrap()
                        .as_ptr(),
                ),
            );

            let bad_actor_json = bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event);
            let bad_actor_json_str = CStr::from_ptr(bad_actor_json).to_str().unwrap();

            // Check our JSON string has a few expected fields
            let final_json: Value = serde_json::from_str(bad_actor_json_str).unwrap();
            dbg!(&final_json["app_name"]);
            assert_eq!(final_json["app_name"], "sentrypeer");

            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
    }

    #[test]
    fn test_json_to_bad_actor_rs() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let json_str = r#"{
                "app_name": "sentrypeer",
                "app_version": "0.1.0",
                "event_timestamp": "blah",
                "event_uuid": "460f30e4-ce1d-4d53-9004-dd40a1c4abc9",
                "created_by_node_id": "350f30e4-ce1d-4d53-9004-dd40a1c4abc8",
                "collected_method": "responsive",
                "transport_type": "UDP",
                "source_ip": "127.0.0.1",
                "destination_ip": "127.0.0.1",
                "called_number": "1234",
                "sip_method": "INVITE",
                "sip_user_agent": "SIPp",
                "sip_message": "INVITE"
            }"#;

            let bad_actor_event = json_to_bad_actor_rs(
                sentrypeer_c_config,
                CString::new(json_str).unwrap().into_raw(),
            );
            assert_str_eq!(
                CStr::from_ptr((*bad_actor_event).collected_method)
                    .to_str()
                    .unwrap(),
                String::from("responsive")
            );

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
    }

    #[test]
    fn test_json_log_bad_actor_rs() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let bad_actor_event = bad_actor_new(
                util_duplicate_string(CString::new("SIP Message").unwrap().as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1").unwrap().as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1").unwrap().as_ptr()),
                util_duplicate_string(CString::new("441234512346").unwrap().as_ptr()),
                util_duplicate_string(CString::new("INVITE").unwrap().as_ptr()),
                util_duplicate_string(CString::new("TLS").unwrap().as_ptr()),
                util_duplicate_string(CString::new("SIPp").unwrap().as_ptr()),
                util_duplicate_string(CString::new("responsive").unwrap().as_ptr()),
                util_duplicate_string(
                    CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")
                        .unwrap()
                        .as_ptr(),
                ),
            );

            let result = json_log_bad_actor_rs(sentrypeer_c_config, bad_actor_event);
            assert_eq!(result, libc::EXIT_SUCCESS);

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
    }
}
