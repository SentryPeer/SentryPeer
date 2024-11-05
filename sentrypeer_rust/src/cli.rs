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
    #[arg(short, value_name = "BOOTSTRAP_NODE")]
    bootstrap: Option<String>,

    /// Set OAuth 2 client ID or use SENTRYPEER_OAUTH2_CLIENT_ID env to get a Bearer token for WebHook
    #[arg(short = 'i')]
    client_id: Option<String>,

    /// Set OAuth 2 client secret or use SENTRYPEER_OAUTH2_CLIENT_SECRET env to get a Bearer token for WebHook
    #[arg(short)]
    client_secret: Option<String>,

    /// Enable RESTful API mode or use SENTRYPEER_API env
    #[arg(short)]
    api: bool,

    /// Set WebHook URL for bad actor json POSTs or use SENTRYPEER_WEBHOOK_URL env
    #[arg(short)]
    webhook_url: Option<String>,

    /// Enable SIP responsive mode or use SENTRYPEER_SIP_RESPONSIVE env
    #[arg(short = 'r')]
    responsive: bool,

    /// Disable SIP mode completely or use SENTRYPEER_SIP_DISABLE env
    #[arg(short = 'R')]
    unresponsive: bool,

    /// Set 'sentrypeer_json.log' location or use SENTRYPEER_JSON_LOG_FILE env
    #[arg(short = 'l')]
    json_log_file: Option<PathBuf>,

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
/// Shutdown the tokio runtime.
#[no_mangle]
pub(crate) unsafe extern "C" fn process_cli_rs(sentrypeer_c_config: *mut sentrypeer_config) -> i32 {
    let args = Args::parse();

    (*sentrypeer_c_config).db_file = args.db_file.map(|p| p.to_string_lossy().to_string());
    
    if args.json {
        (*sentrypeer_c_config).json_log_mode = true;
        
        let json_log_file = args.json_log_file.unwrap_or_else(|| PathBuf::from("sentrypeer_json.log"));
        (*sentrypeer_c_config).json_log_file = Some(json_log_file.to_string_lossy().to_string());
    }
    
    (*sentrypeer_c_config).p2p_dht_mode = args.p2p;
    
    
    (*sentrypeer_c_config).p2p_bootstrap_node = args.bootstrap.map(|s| s.to_string());
    (*sentrypeer_c_config).oauth2_client_id = args.client_id.map(|s| s.to_string());
    (*sentrypeer_c_config).oauth2_client_secret = args.client_secret.map(|s| s.to_string());
    (*sentrypeer_c_config).api_mode = args.api;
    (*sentrypeer_c_config).webhook_url = args.webhook_url.map(|s| s.to_string());
    (*sentrypeer_c_config).sip_responsive_mode = args.responsive;
    (*sentrypeer_c_config).sip_mode = args.unresponsive;
    (*sentrypeer_c_config).json_log_file = args.json_log_file.map(|p| p.to_string_lossy().to_string());
    (*sentrypeer_c_config).syslog_mode = args.syslog;
    (*sentrypeer_c_config).verbose_mode = args.verbose;
    (*sentrypeer_c_config).debug_mode = args.debug;

    libc::EXIT_SUCCESS
}
