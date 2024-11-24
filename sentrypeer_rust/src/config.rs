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
use crate::sentrypeer_config;
use confy::get_configuration_file_path;
use pki_types::{CertificateDer, PrivateKeyDer};
use rcgen::{generate_simple_self_signed, CertifiedKey};
use rustls_pemfile::{certs, private_key};
use serde::{Deserialize, Serialize};
use std::ffi::CStr;
use std::fs::File;
use std::io;
use std::io::{BufReader, ErrorKind};
use std::path::{Path, PathBuf};

#[derive(Debug, Copy, Clone)]
pub struct SentryPeerConfig {
    pub(crate) p: *mut sentrypeer_config,
}
unsafe impl Send for SentryPeerConfig {}
unsafe impl Sync for SentryPeerConfig {}

#[derive(Debug, Serialize, Deserialize)]
pub struct Config {
    pub cert: PathBuf,
    pub key: PathBuf,
    pub tls_listen_address: String,
}

pub(crate) fn config_from_env(config: Config) -> Result<Config, Box<dyn std::error::Error>> {
    // Try to load SENTRYPEER_CERT, SENTRYPEER_KEY and SENTRYPEER_TLS_LISTEN_ADDRESS from our env
    let cert = std::env::var("SENTRYPEER_CERT")
        .unwrap_or_else(|_| config.cert.into_os_string().into_string().unwrap());
    let key = std::env::var("SENTRYPEER_KEY")
        .unwrap_or_else(|_| config.key.into_os_string().into_string().unwrap());
    let tls_listen_address = std::env::var("SENTRYPEER_TLS_LISTEN_ADDRESS")
        .unwrap_or_else(|_| config.tls_listen_address.clone());

    let config = Config {
        cert: PathBuf::from(cert),
        key: PathBuf::from(key),
        tls_listen_address,
    };

    Ok(config)
}

pub(crate) fn config_from_cli(
    config: Config,
    sentrypeer_c_config: *mut sentrypeer_config,
) -> Result<Config, Box<dyn std::error::Error>> {
    // We just test one, as the cert, key and listen address are all "required" by clap-rs
    if unsafe { (*sentrypeer_c_config).tls_cert_file.is_null() } {
        return Ok(config);
    }

    let tls_cert_file = unsafe {
        CStr::from_ptr((*sentrypeer_c_config).tls_cert_file)
            .to_str()
            .unwrap()
    };
    let tls_key_file = unsafe {
        CStr::from_ptr((*sentrypeer_c_config).tls_key_file)
            .to_str()
            .unwrap()
    };

    let tls_listen_address = unsafe {
        CStr::from_ptr((*sentrypeer_c_config).tls_listen_address)
            .to_str()
            .unwrap()
    };

    let cert = PathBuf::from(tls_cert_file);
    let key = PathBuf::from(tls_key_file);

    let config = Config {
        cert,
        key,
        tls_listen_address: tls_listen_address.to_string(),
    };

    Ok(config)
}

pub(crate) fn load_certs(path: &Path) -> io::Result<Vec<CertificateDer<'static>>> {
    certs(&mut BufReader::new(File::open(path)?)).collect()
}

pub(crate) fn load_key(path: &Path) -> io::Result<PrivateKeyDer<'static>> {
    private_key(&mut BufReader::new(File::open(path)?))?.ok_or(io::Error::new(
        ErrorKind::Other,
        "no private key found".to_string(),
    ))
}

/// `Config` implements `Default`
impl Default for Config {
    fn default() -> Self {
        Self {
            cert: "cert.pem".into(),
            key: "key.pem".into(),
            tls_listen_address: "0.0.0.0:5061".into(),
        }
    }
}

pub fn load_file(debug: bool, verbose: bool) -> Result<Config, confy::ConfyError> {
    if debug || verbose {
        let config_file_location = get_configuration_file_path("sentrypeer", None)?;
        println!("Loading config file from: {:?}", config_file_location);
    }

    let cfg = confy::load("sentrypeer", None)?;
    Ok(cfg)
}

/// Ask to create a new TLS cert and key using rcgen
pub fn create_tls_cert_and_key() -> i32 {
    // Prompt Y/N
    let mut input = String::new();
    println!("Would you like to create a new TLS cert and key? [Y/n]");
    std::io::stdin().read_line(&mut input).unwrap();
    let input = input.trim().to_lowercase();

    if input == "y" || input == "yes" {
        // Create a new TLS cert and key
        let CertifiedKey { cert, key_pair } =
            generate_simple_self_signed(vec!["localhost".to_string()]).unwrap();

        std::fs::write("cert.pem", cert.pem()).expect("Unable to write cert.pem");
        std::fs::write("key.pem", key_pair.serialize_pem()).expect("Unable to write key.pem");

        println!("New TLS cert and key created.");
        return libc::EXIT_SUCCESS;
    };

    if input == "n" || input == "no" {
        println!("Please provide a valid TLS cert and key file");
    } else {
        println!("Invalid input.");
    }
    libc::EXIT_SUCCESS
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    // https://doc.rust-lang.org/reference/attributes/testing.html#the-ignore-attribute
    #[test]
    fn test_config_from_env() {
        let mut config = Config {
            cert: PathBuf::from("cert.pem"),
            key: PathBuf::from("key.pem"),
            tls_listen_address: "0.0.0.0:5061".into(),
        };
        config = config_from_env(config).unwrap();
        assert_eq!(config.cert, PathBuf::from("cert.pem"));
        assert_eq!(config.key, PathBuf::from("key.pem"));
        assert_eq!(config.tls_listen_address, "0.0.0.0:5061");
    }

    #[test]
    #[ignore = "not yet implemented"]
    fn test_load_certs() {
        let path = Path::new("tests/certs/cert.pem");
        let certs = load_certs(path).expect("Failed to load certs");
        assert_eq!(certs.len(), 1);
    }

    fn setup_config_file() {
        let cfg = Config {
            cert: "cert.pem".into(),
            key: "key.pem".into(),
            tls_listen_address: "0.0.0.0:5061".into(),
        };
        confy::store("sentrypeer", None, cfg).unwrap();
    }

    #[test]
    fn test_config_default() {
        let cfg = Config::default();
        assert_eq!(cfg.cert, PathBuf::from("cert.pem"));
        assert_eq!(cfg.key, PathBuf::from("key.pem"));
        assert_eq!(cfg.tls_listen_address, "0.0.0.0:5061");
    }

    #[test]
    fn test_load_file() {
        setup_config_file();

        let cfg: Config = load_file(true, false).unwrap();
        assert_eq!(cfg.cert, PathBuf::from("cert.pem"));
        assert_eq!(cfg.key, PathBuf::from("key.pem"));
        assert_eq!(cfg.tls_listen_address, "0.0.0.0:5061");
    }

    #[test]
    fn test_load_file_error() {
        let cfg: Result<Config, confy::ConfyError> = load_file(true, false);
        assert!(cfg.is_ok());
    }

    #[test]
    fn test_load_file_and_save() {
        let cfg: Config = load_file(true, false).unwrap();
        assert_eq!(cfg.cert, PathBuf::from("cert.pem"));
        assert_eq!(cfg.key, PathBuf::from("key.pem"));
        assert_eq!(cfg.tls_listen_address, "0.0.0.0:5061");
        let cfg = Config {
            cert: "cert2.pem".into(),
            key: "key2.pem".into(),
            tls_listen_address: "0.0.0.0:5062".into(),
        };
        confy::store("sentrypeer", None, cfg).unwrap();

        // Reset to original
        setup_config_file();
    }
}
