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
use crate::cli;
use libc::c_char;
use std::ffi::{CStr, CString};
use std::fs::OpenOptions;
use std::io::{BufWriter, Write};
// Our C FFI functions
use crate::{
    PACKAGE_NAME, PACKAGE_VERSION, SENTRYPEER_OAUTH2_AUDIENCE, SENTRYPEER_OAUTH2_GRANT_TYPE,
    SENTRYPEER_OAUTH2_TOKEN_URL, bad_actor, bad_actor_new, free_oauth2_access_token,
    sentrypeer_config, util_duplicate_string,
};

fn c_char_ptr_to_str<'a>(ptr: *const c_char) -> &'a str {
    if ptr.is_null() {
        ""
    } else {
        unsafe { CStr::from_ptr(ptr) }.to_str().unwrap_or("")
    }
}

fn json_val_to_cstring(val: &serde_json::Value, key: &str) -> CString {
    let s = val.get(key).and_then(|v| v.as_str()).unwrap_or("not_found");
    CString::new(s).unwrap_or_else(|_| {
        CString::new("not_found").unwrap_or_default()
    })
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn bad_actor_to_json_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> *mut c_char {
    let debug_mode = if sentrypeer_c_config.is_null() {
        false
    } else {
        (unsafe { *sentrypeer_c_config }).debug_mode
    };
    let verbose_mode = if sentrypeer_c_config.is_null() {
        false
    } else {
        (unsafe { *sentrypeer_c_config }).verbose_mode
    };

    if bad_actor_event.is_null() {
        return std::ptr::null_mut();
    }

    // Make our JSON by hand with serde_json::json!()
    let json = serde_json::json!({
        "app_name": cli::cstr_to_string(PACKAGE_NAME),
        "app_version": cli::cstr_to_string(PACKAGE_VERSION),
        "event_timestamp": c_char_ptr_to_str(unsafe { (*bad_actor_event).event_timestamp }),
        "event_uuid": c_char_ptr_to_str(unsafe { (*bad_actor_event).event_uuid }),
        "created_by_node_id": c_char_ptr_to_str(unsafe { (*bad_actor_event).created_by_node_id }),
        "collected_method": c_char_ptr_to_str(unsafe { (*bad_actor_event).collected_method }),
        "transport_type": c_char_ptr_to_str(unsafe { (*bad_actor_event).transport_type }),
        "source_ip": c_char_ptr_to_str(unsafe { (*bad_actor_event).source_ip }),
        "destination_ip": c_char_ptr_to_str(unsafe { (*bad_actor_event).destination_ip }),
        "called_number": c_char_ptr_to_str(unsafe { (*bad_actor_event).called_number }),
        "sip_method": c_char_ptr_to_str(unsafe { (*bad_actor_event).method }),
        "sip_user_agent": c_char_ptr_to_str(unsafe { (*bad_actor_event).user_agent }),
        "sip_message": c_char_ptr_to_str(unsafe { (*bad_actor_event).sip_message }),
    });

    if debug_mode || verbose_mode {
        eprintln!("Bad actor in JSON format: {:?}", json.to_string());
    }

    // Return the JSON as a C string which must be freed
    let json_string = json.to_string();
    match CString::new(json_string) {
        Ok(c_str) => c_str.into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn free_json_rs(json: *mut c_char) {
    if json.is_null() {
        return;
    }

    let _ = unsafe { CString::from_raw(json) };
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_to_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    json_to_convert: *const c_char,
) -> *mut bad_actor {
    let debug_mode = if sentrypeer_c_config.is_null() {
        false
    } else {
        (unsafe { *sentrypeer_c_config }).debug_mode
    };
    let verbose_mode = if sentrypeer_c_config.is_null() {
        false
    } else {
        (unsafe { *sentrypeer_c_config }).verbose_mode
    };

    if json_to_convert.is_null() {
        return std::ptr::null_mut();
    }

    let json_str = match unsafe { CStr::from_ptr(json_to_convert) }.to_str() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Invalid UTF-8 in JSON string: {e}");
            return std::ptr::null_mut();
        }
    };

    if debug_mode || verbose_mode {
        eprintln!("JSON to convert to a bad_actor: {json_str:?}");
    }

    let v: serde_json::Value = match serde_json::from_str(json_str) {
        Ok(val) => val,
        Err(e) => {
            eprintln!("Failed to parse JSON string: {e}");
            return std::ptr::null_mut();
        }
    };

    unsafe {
        bad_actor_new(
            json_val_to_cstring(&v, "sip_message").into_raw(),
            json_val_to_cstring(&v, "source_ip").into_raw(),
            json_val_to_cstring(&v, "destination_ip").into_raw(),
            json_val_to_cstring(&v, "called_number").into_raw(),
            json_val_to_cstring(&v, "sip_method").into_raw(),
            json_val_to_cstring(&v, "transport_type").into_raw(),
            json_val_to_cstring(&v, "sip_user_agent").into_raw(),
            json_val_to_cstring(&v, "collected_method").into_raw(),
            json_val_to_cstring(&v, "created_by_node_id").into_raw(),
        )
    }
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_log_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> i32 {
    if sentrypeer_c_config.is_null() || bad_actor_event.is_null() {
        return libc::EXIT_FAILURE;
    }

    let log_file_ptr = (unsafe { *sentrypeer_c_config }).json_log_file;
    if log_file_ptr.is_null() {
        return libc::EXIT_FAILURE;
    }

    let log_file_name = match unsafe { CStr::from_ptr(log_file_ptr) }.to_str() {
        Ok(name) => name,
        Err(e) => {
            eprintln!("Invalid UTF-8 in JSON log file name: {e}");
            return libc::EXIT_FAILURE;
        }
    };

    let json = unsafe { bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event) };
    if json.is_null() {
        return libc::EXIT_FAILURE;
    }
    let json_str = match unsafe { CStr::from_ptr(json) }.to_str() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Invalid UTF-8 in generated JSON: {e}");
            unsafe { free_json_rs(json) };
            return libc::EXIT_FAILURE;
        }
    };

    let json_log_file = match OpenOptions::new()
        .append(true)
        .create(true)
        .open(log_file_name)
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Could not open JSON log file: {e}");
            unsafe { free_json_rs(json) };
            return libc::EXIT_FAILURE;
        }
    };

    let mut buf = BufWriter::new(json_log_file);
    let json_str = format!("{json_str}\n");

    let write_result = match buf.write_all(json_str.as_bytes()) {
        Ok(()) => libc::EXIT_SUCCESS,
        Err(e) => {
            eprintln!("Error writing to JSON log file: {e}");
            libc::EXIT_FAILURE
        }
    };

    unsafe { free_json_rs(json) };
    write_result
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_http_post_bad_actor_rs(
    sentrypeer_c_config: *mut sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> i32 {
    if sentrypeer_c_config.is_null() || bad_actor_event.is_null() {
        return libc::EXIT_FAILURE;
    }

    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    let json = unsafe { bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event) };
    if json.is_null() {
        return libc::EXIT_FAILURE;
    }
    let json_str = match unsafe { CStr::from_ptr(json) }.to_str() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Invalid UTF-8 in generated JSON: {e}");
            unsafe { free_json_rs(json) };
            return libc::EXIT_FAILURE;
        }
    };

    // We already have an access token, so we set it in our header
    if (unsafe { *sentrypeer_c_config }).oauth2_mode {
        if (unsafe { *sentrypeer_c_config })
            .oauth2_access_token
            .is_null()
        {
            if debug_mode || verbose_mode {
                eprintln!("Requesting OAuth2 Bearer Token");
            }

            let client_id = (unsafe { *sentrypeer_c_config }).oauth2_client_id;
            let client_id_str = c_char_ptr_to_str(client_id);

            let client_secret = (unsafe { *sentrypeer_c_config }).oauth2_client_secret;
            let client_secret_str = c_char_ptr_to_str(client_secret);

            let audience = cli::cstr_to_string(SENTRYPEER_OAUTH2_AUDIENCE);
            let grant_type = cli::cstr_to_string(SENTRYPEER_OAUTH2_GRANT_TYPE);

            let json_client_creds = serde_json::json!({
                "client_id": client_id_str,
                "client_secret": client_secret_str,
                "audience": audience,
                "grant_type": grant_type
            });

            if debug_mode || verbose_mode {
                eprintln!("Client credentials in JSON format: {json_client_creds}");
            }

            // Send the request to get the access token
            let url = cli::cstr_to_string(SENTRYPEER_OAUTH2_TOKEN_URL);
            let client = reqwest::blocking::Client::new();
            let res = match client
                .post(&url)
                .header("Content-Type", "application/json")
                .body(json_client_creds.to_string())
                .send()
            {
                Ok(r) => r,
                Err(e) => {
                    eprintln!("OAuth2 Token Request POSTing failed: {e}");
                    unsafe { free_json_rs(json) };
                    return libc::EXIT_FAILURE;
                }
            };

            if res.status() != 200 {
                eprintln!(
                    "OAuth2 Token Request POSTing failed: HTTP response code: {:?}",
                    res.status()
                );
                unsafe { free_json_rs(json) };
                return libc::EXIT_FAILURE;
            }

            let access_token_json = match res.json::<serde_json::Value>() {
                Ok(val) => val,
                Err(e) => {
                    eprintln!("Failed to parse JSON response: {e}");
                    unsafe { free_json_rs(json) };
                    return libc::EXIT_FAILURE;
                }
            };

            let Some(access_token) = access_token_json
                .get("access_token")
                .and_then(|v| v.as_str())
            else {
                eprintln!("Failed to get access_token from JSON response");
                unsafe { free_json_rs(json) };
                return libc::EXIT_FAILURE;
            };

            if debug_mode || verbose_mode {
                eprintln!("Got access_token: {access_token:?}");
            }

            let Ok(access_token_c_str) = CString::new(access_token) else {
                eprintln!("access_token is not a valid C string");
                unsafe { free_json_rs(json) };
                return libc::EXIT_FAILURE;
            };

            unsafe {
                (*sentrypeer_c_config).oauth2_access_token =
                    util_duplicate_string(access_token_c_str.as_ptr());
            };

            if debug_mode || verbose_mode {
                eprintln!("Retrieved access_token from config: {:?}", unsafe {
                    CStr::from_ptr((*sentrypeer_c_config).oauth2_access_token)
                        .to_str()
                        .unwrap_or("")
                });
            }
        }

        // Now we have an access token, we can send the JSON to the webhook URL
        let access_token = (unsafe { *sentrypeer_c_config }).oauth2_access_token;
        let access_token_str = c_char_ptr_to_str(access_token);

        let url = (unsafe { *sentrypeer_c_config }).webhook_url;
        let url_str = c_char_ptr_to_str(url);

        let client = reqwest::blocking::Client::new();
        let res = match client
            .post(url_str)
            .header("Authorization", format!("Bearer {access_token_str}"))
            .header("Content-Type", "application/json")
            .body(json_str.to_string())
            .send()
        {
            Ok(r) => r,
            Err(e) => {
                eprintln!("WebHook POSTing failed: {e}");
                unsafe { free_json_rs(json) };
                return libc::EXIT_FAILURE;
            }
        };

        if res.status() != 200 && res.status() != 201 {
            let res_status = res.status();
            unsafe { free_json_rs(json) };
            return if res_status == 401 || res_status == 403 {
                // The token has probably expired (lasts 86400 seconds - 1 day)
                // Let's reset it and get a new one
                if debug_mode || verbose_mode {
                    eprintln!("OAuth2 access token expired, resetting.");
                }
                unsafe { free_oauth2_access_token(sentrypeer_c_config) };

                if unsafe { json_http_post_bad_actor_rs(sentrypeer_c_config, bad_actor_event) }
                    != libc::EXIT_SUCCESS
                {
                    eprintln!("Failed to POST bad actor.");
                    libc::EXIT_FAILURE
                } else {
                    eprintln!(
                        "WebHook POSTing failed: HTTP response code: {res_status:?}"
                    );
                    libc::EXIT_FAILURE
                }
            } else {
                eprintln!(
                    "WebHook POSTing failed: HTTP response code: {res_status:?}"
                );
                libc::EXIT_FAILURE
            };
        }

        if debug_mode || verbose_mode {
            eprintln!(
                "WebHook POSTing succeeded: HTTP response code {:?}",
                res.status()
            );
        }

        unsafe { free_json_rs(json) };
        return libc::EXIT_SUCCESS;
    }

    unsafe { free_json_rs(json) };
    libc::EXIT_SUCCESS
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{
        PACKAGE_VERSION, bad_actor_destroy, bad_actor_new, http_daemon_init, http_daemon_stop,
        sentrypeer_config_new, util_duplicate_string,
    };
    use pretty_assertions::assert_str_eq;
    use reqwest::blocking::Client;
    use serde_json::Value;

    #[test]
    fn test_bad_actor_to_json_rs() -> Result<(), Box<dyn std::error::Error>> {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let bad_actor_event = bad_actor_new(
                util_duplicate_string(CString::new("SIP Message")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("441234512346")?.as_ptr()),
                util_duplicate_string(CString::new("INVITE")?.as_ptr()),
                util_duplicate_string(CString::new("TLS")?.as_ptr()),
                util_duplicate_string(CString::new("SIPp")?.as_ptr()),
                util_duplicate_string(CString::new("responsive")?.as_ptr()),
                util_duplicate_string(
                    CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")?
                        .as_ptr(),
                ),
            );

            let bad_actor_json = bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event);
            let bad_actor_json_str = CStr::from_ptr(bad_actor_json).to_str()?;

            // Check our JSON string has a few expected fields
            let final_json: Value = serde_json::from_str(bad_actor_json_str)?;
            dbg!(&final_json["app_name"]);
            assert_eq!(final_json["app_name"], "sentrypeer");

            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
            free_json_rs(bad_actor_json);
        }
        Ok(())
    }

    #[test]
    fn test_json_to_bad_actor_rs() -> Result<(), Box<dyn std::error::Error>> {
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
                CString::new(json_str)?.into_raw(),
            );
            assert_str_eq!(
                CStr::from_ptr((*bad_actor_event).collected_method)
                    .to_str()?,
                String::from("responsive")
            );

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
        Ok(())
    }

    #[test]
    fn test_json_log_bad_actor_rs() -> Result<(), Box<dyn std::error::Error>> {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            let bad_actor_event = bad_actor_new(
                util_duplicate_string(CString::new("SIP Message")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("441234512346")?.as_ptr()),
                util_duplicate_string(CString::new("INVITE")?.as_ptr()),
                util_duplicate_string(CString::new("TLS")?.as_ptr()),
                util_duplicate_string(CString::new("SIPp")?.as_ptr()),
                util_duplicate_string(CString::new("responsive")?.as_ptr()),
                util_duplicate_string(
                    CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")?
                        .as_ptr(),
                ),
            );

            let result = json_log_bad_actor_rs(sentrypeer_c_config, bad_actor_event);
            assert_eq!(result, libc::EXIT_SUCCESS);

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
        Ok(())
    }

    #[test]
    fn test_http_api_health_check_version() -> Result<(), Box<dyn std::error::Error>> {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            assert_eq!(http_daemon_init(sentrypeer_c_config), libc::EXIT_SUCCESS);

            let health_check_endpoint = "http://localhost:8082/health-check";
            let client = Client::new();
            let res = client
                .get(health_check_endpoint)
                .header("Content-Type", "application/json")
                .send()?;

            assert_eq!(res.status(), 200);
            let body = res.text()?;
            dbg!(&body);
            assert!(body.contains(&cli::cstr_to_string(PACKAGE_VERSION)));

            assert_eq!(http_daemon_stop(sentrypeer_c_config), libc::EXIT_SUCCESS);
        }
        Ok(())
    }

    #[test]
    fn test_json_http_post_bad_actor_rs() -> Result<(), Box<dyn std::error::Error>> {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            (*sentrypeer_c_config).oauth2_mode = true;
            (*sentrypeer_c_config).debug_mode = true;

            let bad_actor_event = bad_actor_new(
                util_duplicate_string(CString::new("SIP Message")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("127.0.0.1")?.as_ptr()),
                util_duplicate_string(CString::new("441234512346")?.as_ptr()),
                util_duplicate_string(CString::new("INVITE")?.as_ptr()),
                util_duplicate_string(CString::new("TLS")?.as_ptr()),
                util_duplicate_string(CString::new("SIPp")?.as_ptr()),
                util_duplicate_string(CString::new("responsive")?.as_ptr()),
                util_duplicate_string(
                    CString::new("460f30e4-ce1d-4d53-9004-dd40a1c4abc9")?
                        .as_ptr(),
                ),
            );

            let result = json_http_post_bad_actor_rs(sentrypeer_c_config, bad_actor_event);
            assert_eq!(result, libc::EXIT_FAILURE);

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
        Ok(())
    }
}
