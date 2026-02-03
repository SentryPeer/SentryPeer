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

#[unsafe(no_mangle)]
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
        if unsafe { (*bad_actor_event).event_timestamp.is_null() } {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).event_timestamp).to_str().unwrap() }
        },
        "event_uuid":
        if unsafe { (*bad_actor_event).event_uuid.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).event_uuid).to_str().unwrap() }
        },
        "created_by_node_id":
        if unsafe { (*bad_actor_event).created_by_node_id.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).created_by_node_id).to_str().unwrap() }
        },
        "collected_method":
        if unsafe { (*bad_actor_event).collected_method.is_null()} {
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).collected_method).to_str().unwrap() }
        },
        "transport_type":
        if unsafe { (*bad_actor_event).transport_type.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).transport_type).to_str().unwrap() }
        },
        "source_ip":
        if unsafe { (*bad_actor_event).source_ip.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).source_ip).to_str().unwrap() }
        },
        "destination_ip":
        if unsafe { (*bad_actor_event).destination_ip.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).destination_ip).to_str().unwrap() }
        },
        "called_number":
        if unsafe { (*bad_actor_event).called_number.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).called_number).to_str().unwrap() }
        },
        "sip_method":
        if unsafe { (*bad_actor_event).method.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).method).to_str().unwrap() }
        },
        "sip_user_agent":
        if unsafe { (*bad_actor_event).user_agent.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).user_agent).to_str().unwrap() }
        },
        "sip_message":
        if unsafe { (*bad_actor_event).sip_message.is_null() }{
            ""
        } else {
            unsafe { CStr::from_ptr((*bad_actor_event).sip_message).to_str().unwrap() }
        },
    });

    if debug_mode || verbose_mode {
        eprintln!("Bad actor in JSON format: {:?}", json.to_string());
    }

    // Return the JSON as a C string which must be freed
    CString::new(json.to_string()).unwrap().into_raw()
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn free_json_rs(json: *mut c_char) {
    unsafe {
        if json.is_null() {
            return;
        }

        let _ = CString::from_raw(json);
    }
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_to_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    json_to_convert: *const c_char,
) -> *mut bad_actor {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    let json_str = unsafe { CStr::from_ptr(json_to_convert).to_str().unwrap() };
    if debug_mode || verbose_mode {
        eprintln!("JSON to convert to a bad_actor: {json_str:?}");
    }

    let v: serde_json::Value = serde_json::from_str(json_str).unwrap();

    unsafe {
        bad_actor_new(
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
        )
    }
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_log_bad_actor_rs(
    sentrypeer_c_config: *const sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> i32 {
    let log_file_name = unsafe {
        CStr::from_ptr((*sentrypeer_c_config).json_log_file)
            .to_str()
            .unwrap()
    };

    let json = unsafe { bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event) };
    let json_str = unsafe { CStr::from_ptr(json).to_str().unwrap() };

    let json_log_file = match OpenOptions::new()
        .append(true)
        .create(true)
        .open(log_file_name)
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("Could not open JSON log file: {e}");
            return libc::EXIT_FAILURE;
        }
    };

    let mut buf = BufWriter::new(json_log_file);
    let json_str = format!("{json_str}\n");

    match buf.write_all(json_str.as_bytes()) {
        Ok(_) => (),
        Err(e) => {
            eprintln!("Error writing to JSON log file: {e}");
            return libc::EXIT_FAILURE;
        }
    }

    libc::EXIT_SUCCESS
}

#[unsafe(no_mangle)]
pub(crate) unsafe extern "C" fn json_http_post_bad_actor_rs(
    sentrypeer_c_config: *mut sentrypeer_config,
    bad_actor_event: *const bad_actor,
) -> i32 {
    let debug_mode = (unsafe { *sentrypeer_c_config }).debug_mode;
    let verbose_mode = (unsafe { *sentrypeer_c_config }).verbose_mode;

    let json = unsafe { bad_actor_to_json_rs(sentrypeer_c_config, bad_actor_event) };
    let json_str = unsafe { CStr::from_ptr(json).to_str().unwrap() };

    // We already have an access token, so we set it in our header
    if (unsafe { *sentrypeer_c_config }).oauth2_mode {
        if (unsafe { *sentrypeer_c_config })
            .oauth2_access_token
            .is_null()
        {
            if debug_mode || verbose_mode {
                eprintln!("Requesting OAuth2 Bearer Token");
            }

            // Create JSON string for the request body
            // {
            //    "client_id": "your_client_id",
            //    "client_secret": "your_client_secret",
            //    "audience": "your_audience",
            //    "grant_type": "client_credentials"
            // }
            let client_id = (unsafe { *sentrypeer_c_config }).oauth2_client_id;
            let client_id_str = unsafe { CStr::from_ptr(client_id).to_str().unwrap() };

            let client_secret = (unsafe { *sentrypeer_c_config }).oauth2_client_secret;
            let client_secret_str = unsafe { CStr::from_ptr(client_secret).to_str().unwrap() };

            let audience = &cli::cstr_to_string(SENTRYPEER_OAUTH2_AUDIENCE);
            let grant_type = &cli::cstr_to_string(SENTRYPEER_OAUTH2_GRANT_TYPE);

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
            let res = client
                .post(&url)
                .header("Content-Type", "application/json")
                .body(json_client_creds.to_string())
                .send()
                .expect("OAuth2 Token Request POSTing failed.");

            if res.status() != 200 {
                eprintln!(
                    "OAuth2 Token Request POSTing failed: HTTP response code: {:?}",
                    res.status()
                );
                return libc::EXIT_FAILURE;
            }

            let access_token_json = res
                .json::<serde_json::Value>()
                .expect("Failed to parse JSON response");

            let access_token = access_token_json
                .get("access_token")
                .expect("Failed to get access_token from JSON response")
                .as_str()
                .expect("access_token is not a string");

            if debug_mode || verbose_mode {
                eprintln!("Got access_token: {access_token:?}");
            }

            let access_token_c_str =
                CString::new(access_token).expect("access_token is not a string");

            unsafe {
                (*sentrypeer_c_config).oauth2_access_token =
                    util_duplicate_string(access_token_c_str.as_ptr())
            };

            if debug_mode || verbose_mode {
                eprintln!("Retrieved access_token from config: {:?}", unsafe {
                    CStr::from_ptr((*sentrypeer_c_config).oauth2_access_token)
                        .to_str()
                        .unwrap()
                });
            }
        }

        // Now we have an access token, we can send the JSON to the webhook URL
        let access_token = (unsafe { *sentrypeer_c_config }).oauth2_access_token;
        let access_token_str = unsafe { CStr::from_ptr(access_token).to_str().unwrap() };

        let url = (unsafe { *sentrypeer_c_config }).webhook_url;
        let url_str = unsafe { CStr::from_ptr(url).to_str().unwrap() };

        let client = reqwest::blocking::Client::new();
        let res = client
            .post(url_str)
            .header("Authorization", format!("Bearer {access_token_str}"))
            .header("Content-Type", "application/json")
            .body(json_str.to_string())
            .send()
            .expect("WebHook POSTing failed.");

        if res.status() != 200 && res.status() != 201 {
            return if res.status() == 401 || res.status() == 403 {
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
                        "WebHook POSTing failed: HTTP response code: {:?}",
                        res.status()
                    );
                    libc::EXIT_FAILURE
                }
            } else {
                eprintln!(
                    "WebHook POSTing failed: HTTP response code: {:?}",
                    res.status()
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

        return libc::EXIT_SUCCESS;
    }

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

    #[test]
    fn test_http_api_health_check_version() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            if http_daemon_init(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                panic!("Failed to start the HTTP server");
            }

            let health_check_endpoint = "http://localhost:8082/health-check";
            let client = Client::new();
            let res = client
                .get(health_check_endpoint)
                .header("Content-Type", "application/json")
                .send()
                .expect("Failed to send request");

            assert_eq!(res.status(), 200);
            let body = res.text().expect("Failed to read response");
            dbg!(&body);
            assert!(body.contains(&cli::cstr_to_string(PACKAGE_VERSION)));

            if http_daemon_stop(sentrypeer_c_config) != libc::EXIT_SUCCESS {
                panic!("Failed to stop the HTTP server");
            }
        }
    }

    #[test]
    fn test_json_http_post_bad_actor_rs() {
        unsafe {
            let sentrypeer_c_config = sentrypeer_config_new();
            (*sentrypeer_c_config).oauth2_mode = true;
            (*sentrypeer_c_config).debug_mode = true;

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

            let result = json_http_post_bad_actor_rs(sentrypeer_c_config, bad_actor_event);
            assert_eq!(result, libc::EXIT_FAILURE);

            // Clean up
            bad_actor_destroy(Box::into_raw(Box::new(bad_actor_event)));
        }
    }
}
