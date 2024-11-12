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
use clap::Parser;
use std::ffi::CString;
use std::path::PathBuf;

// Our C FFI functions
use crate::sentrypeer_config;

/// Protect your SIP Servers from bad actors at https://sentrypeer.org
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Set 'sentrypeer.db' location or use SENTRYPEER_DB_FILE env
    #[arg(short = 'f')]
    db_file: Option<PathBuf>,

    /// Enable json logging or use SENTRYPEER_JSON_LOG env
    #[arg(short)]
    json: bool,

    /// Enable Peer to Peer mode or use SENTRYPEER_PEER_TO_PEER env
    #[arg(short)]
    p2p: bool,

    /// Set Peer to Peer bootstrap node or use SENTRYPEER_BOOTSTRAP_NODE env
    #[arg(short, value_name = "BOOTSTRAP_NODE", requires = "p2p")]
    bootstrap: Option<String>,

    /// Set OAuth 2 client ID or use SENTRYPEER_OAUTH2_CLIENT_ID env to get a Bearer token for WebHook
    #[arg(short = 'i', requires = "client_secret")]
    client_id: Option<String>,

    /// Set OAuth 2 client secret or use SENTRYPEER_OAUTH2_CLIENT_SECRET env to get a Bearer token for WebHook
    #[arg(short, requires = "client_id")]
    client_secret: Option<String>,

    /// Enable RESTful API mode or use SENTRYPEER_API env
    #[arg(short)]
    api: bool,

    /// Set WebHook URL for bad actor json POSTs or use SENTRYPEER_WEBHOOK_URL env
    #[arg(short, requires = "client_id", requires = "client_secret")]
    webhook_url: Option<String>,

    /// Enable SIP responsive mode or use SENTRYPEER_SIP_RESPONSIVE env
    #[arg(short = 'r')]
    responsive: bool,

    /// Disable SIP mode completely or use SENTRYPEER_SIP_DISABLE env
    #[arg(short = 'R')]
    unresponsive: bool,

    /// Set 'sentrypeer_json.log' location or use SENTRYPEER_JSON_LOG_FILE env
    #[arg(short = 'l', requires = "json")]
    json_log_file: Option<PathBuf>,

    /// Disable TLS mode completely or use SENTRYPEER_TLS_DISABLE env
    #[arg(short = 'T')]
    tls_mode: bool,

    /// Set 'cert.pem' location or use SENTRYPEER_CERT env
    #[arg(short = 't', requires = "tls_key_file")]
    tls_cert_file: Option<PathBuf>,

    /// Set 'key.pem' location or use SENTRYPEER_KEY env
    #[arg(short = 'k', requires = "tls_cert_file")]
    tls_key_file: Option<PathBuf>,

    /// Set TLS listen address '127.0.0.1:8088' or use SENTRYPEER_TLS_LISTEN_ADDRESS env
    #[arg(short = 'z', requires = "tls_cert_file", requires = "tls_key_file")]
    tls_listen_address: Option<String>,

    /// Enable syslog logging or use SENTRYPEER_SYSLOG env
    #[arg(short)]
    syslog: bool,

    /// Enable verbose logging or use SENTRYPEER_VERBOSE env
    #[arg(short)]
    verbose: bool,

    /// Enable debug mode or use SENTRYPEER_DEBUG env
    #[arg(short)]
    debug: bool,
}

/// # Safety
///
/// Process the CLI arguments and set the sentrypeer_config struct
#[no_mangle]
pub(crate) unsafe extern "C" fn process_cli_rs(sentrypeer_c_config: *mut sentrypeer_config) -> i32 {
    let args = Args::parse();

    // Set booleans
    (*sentrypeer_c_config).api_mode = args.api;
    (*sentrypeer_c_config).debug_mode = args.debug;
    (*sentrypeer_c_config).p2p_dht_mode = args.p2p;
    if args.unresponsive {
        // Set to true by default in C
        (*sentrypeer_c_config).sip_mode = false;
    }
    if args.tls_mode {
        // Set to true by default in C
        (*sentrypeer_c_config).tls_mode = false;
    }
    (*sentrypeer_c_config).sip_responsive_mode = args.responsive;
    (*sentrypeer_c_config).syslog_mode = args.syslog;
    (*sentrypeer_c_config).verbose_mode = args.verbose;

    // Set strings
    if args.db_file.is_some() {
        let db_file = args.db_file.unwrap();
        (*sentrypeer_c_config).db_file =
            CString::new(db_file.to_str().unwrap()).unwrap().into_raw();
    }

    if args.json {
        (*sentrypeer_c_config).json_log_mode = true;

        if args.json_log_file.is_some() {
            let json_log_file = args.json_log_file.unwrap();
            (*sentrypeer_c_config).json_log_file = CString::new(json_log_file.to_str().unwrap())
                .unwrap()
                .into_raw();
        }
    }

    if args.bootstrap.is_some() {
        (*sentrypeer_c_config).p2p_bootstrap_node =
            CString::new(args.bootstrap.unwrap()).unwrap().into_raw();
    }

    if args.client_id.is_some() {
        (*sentrypeer_c_config).oauth2_client_id =
            CString::new(args.client_id.unwrap()).unwrap().into_raw();
    }

    if args.client_secret.is_some() {
        (*sentrypeer_c_config).oauth2_client_secret = CString::new(args.client_secret.unwrap())
            .unwrap()
            .into_raw();
    }

    if args.webhook_url.is_some() {
        (*sentrypeer_c_config).webhook_url =
            CString::new(args.webhook_url.unwrap()).unwrap().into_raw();
    }

    if args.tls_cert_file.is_some() {
        let tls_cert_file = args.tls_cert_file.unwrap();
        (*sentrypeer_c_config).tls_cert_file = CString::new(tls_cert_file.to_str().unwrap())
            .unwrap()
            .into_raw();
    }

    if args.tls_key_file.is_some() {
        let tls_key_file = args.tls_key_file.unwrap();
        (*sentrypeer_c_config).tls_key_file = CString::new(tls_key_file.to_str().unwrap())
            .unwrap()
            .into_raw();
    }

    if args.tls_listen_address.is_some() {
        (*sentrypeer_c_config).tls_listen_address = CString::new(args.tls_listen_address.unwrap())
            .unwrap()
            .into_raw();
    }

    libc::EXIT_SUCCESS
}

/// # Safety
///
/// Clean up the sentrypeer_config struct after use by clap-rs
#[no_mangle]
pub(crate) unsafe extern "C" fn sentrypeer_cli_destroy_rs(
    sentrypeer_c_config: *mut sentrypeer_config,
) -> i32 {
    // Set strings
    if !(*sentrypeer_c_config).db_file.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).db_file);
        // (*sentrypeer_c_config).db_file =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).json_log_file.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).json_log_file);
        // (*sentrypeer_c_config).json_log_file =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).p2p_bootstrap_node.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).p2p_bootstrap_node);
        // (*sentrypeer_c_config).p2p_bootstrap_node =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).oauth2_client_id.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).oauth2_client_id);
        // (*sentrypeer_c_config).oauth2_client_id =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).oauth2_client_secret.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).oauth2_client_secret);
        // (*sentrypeer_c_config).oauth2_client_secret =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).webhook_url.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).webhook_url);
        // (*sentrypeer_c_config).webhook_url =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).tls_cert_file.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).tls_cert_file);
        // (*sentrypeer_c_config).tls_cert_file =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).tls_key_file.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).tls_key_file);
        // (*sentrypeer_c_config).tls_key_file =
        //     std::ptr::null_mut();
    }

    if !(*sentrypeer_c_config).tls_listen_address.is_null() {
        let _ = CString::from_raw((*sentrypeer_c_config).tls_listen_address);
        // (*sentrypeer_c_config).tls_listen_address =
        //     std::ptr::null_mut();
    }

    libc::EXIT_SUCCESS
}
