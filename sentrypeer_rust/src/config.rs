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
use crate::sentrypeer_config;
use confy::get_configuration_file_path;
use pki_types::{CertificateDer, PrivateKeyDer};
use rcgen::{CertifiedKey, generate_simple_self_signed};
use rustls_pemfile::{certs, private_key};
use serde::{Deserialize, Serialize};
use std::ffi::CStr;
use std::fs::File;
use std::io;
use std::io::BufReader;
use std::path::{Path, PathBuf};

#[derive(Debug, Copy, Clone)]
pub struct SentryPeerConfig {
    pub(crate) p: *mut sentrypeer_config,
}
unsafe impl Send for SentryPeerConfig {}
unsafe impl Sync for SentryPeerConfig {}

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

#[derive(Debug, Serialize, Deserialize)]
pub struct Config {
    pub cert: PathBuf,
    pub key: PathBuf,
    pub tls_listen_address: String,
}

pub(crate) fn config_from_env(config: Config) -> Result<Config, Box<dyn std::error::Error>> {
    // Try to load SENTRYPEER_CERT, SENTRYPEER_KEY and SENTRYPEER_TLS_LISTEN_ADDRESS from our env
    let cert = std::env::var("SENTRYPEER_CERT").or_else(|_| {
        config
            .cert
            .into_os_string()
            .into_string()
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, format!("{:?}", e)))
    })?;

    let key = std::env::var("SENTRYPEER_KEY").or_else(|_| {
        config
            .key
            .into_os_string()
            .into_string()
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, format!("{:?}", e)))
    })?;
    let tls_listen_address = std::env::var("SENTRYPEER_TLS_LISTEN_ADDRESS")
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e.to_string()))
        .or_else(|_| Ok::<String, io::Error>(config.tls_listen_address.clone()))?;

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

    let tls_cert_file = unsafe { CStr::from_ptr((*sentrypeer_c_config).tls_cert_file).to_str()? };
    let tls_key_file = unsafe { CStr::from_ptr((*sentrypeer_c_config).tls_key_file).to_str()? };

    let tls_listen_address =
        unsafe { CStr::from_ptr((*sentrypeer_c_config).tls_listen_address).to_str()? };

    let cert = PathBuf::from(tls_cert_file);
    let key = PathBuf::from(tls_key_file);

    let config = Config {
        cert,
        key,
        tls_listen_address: tls_listen_address.to_string(),
    };

    Ok(config)
}

pub fn load_all_configs(
    sentrypeer_config: SentryPeerConfig,
) -> Result<Config, Box<dyn std::error::Error>> {
    let debug_mode = unsafe { (*sentrypeer_config.p).debug_mode };
    let verbose_mode = unsafe { (*sentrypeer_config.p).verbose_mode };
    let config_file = get_config_file_path(sentrypeer_config)?;

    // Our Configuration file is loaded first, with defaults
    let mut config =
        load_file(debug_mode, verbose_mode, config_file).expect("Failed to load config file");
    // Then our env
    config = config_from_env(config)?;
    // Then our CLI args
    config = config_from_cli(config, sentrypeer_config.p)?;

    Ok(config)
}

pub(crate) fn load_certs(path: &Path) -> io::Result<Vec<CertificateDer<'static>>> {
    certs(&mut BufReader::new(File::open(path)?)).collect()
}

pub(crate) fn load_key(path: &Path) -> io::Result<PrivateKeyDer<'static>> {
    private_key(&mut BufReader::new(File::open(path)?))?
        .ok_or(io::Error::other("no private key found".to_string()))
}

pub fn get_config_file_path(
    sentrypeer_config: SentryPeerConfig,
) -> Result<PathBuf, confy::ConfyError> {
    // If no config file is provided on the cli, we check the env or use the default
    let config_file: PathBuf = if unsafe { (*sentrypeer_config.p).config_file.is_null() } {
        std::env::var("SENTRYPEER_CONFIG_FILE")
            .map(PathBuf::from)
            .or_else(|_| get_configuration_file_path("sentrypeer", None))?
    } else {
        // If a config file is provided on the cli, we use that or the default if it fails
        // to load
        unsafe {
            CStr::from_ptr((*sentrypeer_config.p).config_file)
                .to_str()
                .map(PathBuf::from)
                .or_else(|_| get_configuration_file_path("sentrypeer", None))?
        }
    };

    Ok(config_file)
}

pub fn load_file(
    debug: bool,
    verbose: bool,
    config_file: PathBuf,
) -> Result<Config, confy::ConfyError> {
    // then env
    if debug || verbose {
        eprintln!("Loading config file: {config_file:?}");
    }
    confy::load_path(config_file)
}

/// Ask to create a new TLS cert and key using rcgen
pub fn create_tls_cert_and_key() -> Result<(), Box<dyn std::error::Error>> {
    // Prompt Y/N
    let mut input = String::new();
    println!("Would you like to create a new TLS cert and key? [Y/n]");
    io::stdin().read_line(&mut input)?;
    let input = input.trim().to_lowercase();

    if input == "y" || input == "yes" {
        return match create_certs() {
            Ok(_) => {
                println!("cert.pem and key.pem created successfully");
                Ok(())
            }
            Err(e) => {
                eprintln!("Failed to create TLS cert and key: {e}");
                Err(Box::new(e))
            }
        };
    };

    if input == "n" || input == "no" {
        println!("Please provide a valid TLS cert and key file");
    } else {
        println!("Invalid input.");
    }
    Ok(())
}

// Create a new TLS cert and key usng rcgen
pub fn create_certs() -> io::Result<()> {
    let CertifiedKey { cert, signing_key } =
        generate_simple_self_signed(vec!["localhost".to_string()]).expect("Failed to create cert");

    std::fs::write("cert.pem", cert.pem())?;
    std::fs::write("key.pem", signing_key.serialize_pem())?;

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{sentrypeer_config_destroy, sentrypeer_config_new};
    use serial_test::serial;
    use std::path::PathBuf;

    // https://doc.rust-lang.org/reference/attributes/testing.html#the-ignore-attribute
    #[test]
    #[serial]
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
    #[serial]
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
    #[serial]
    fn test_config_default() {
        let cfg = Config::default();
        assert_eq!(cfg.cert, PathBuf::from("cert.pem"));
        assert_eq!(cfg.key, PathBuf::from("key.pem"));
        assert_eq!(cfg.tls_listen_address, "0.0.0.0:5061");
    }

    #[test]
    #[serial]
    fn test_load_file() {
        setup_config_file();

        let config_file = PathBuf::from("./tests/custom_config.toml");
        let cfg: Config = load_file(true, false, config_file).unwrap();
        assert_eq!(cfg.cert, PathBuf::from("cert.pem"));
        assert_eq!(cfg.key, PathBuf::from("key.pem"));
        assert_eq!(cfg.tls_listen_address, "0.0.0.0:5061");
    }

    #[test]
    #[serial]
    fn test_load_file_error() {
        let cfg: Result<Config, confy::ConfyError> =
            load_file(true, false, PathBuf::from("non_existent_file.toml"));
        assert!(cfg.is_ok());
    }

    #[test]
    #[serial]
    fn test_load_file_and_save() {
        setup_config_file();

        let config_file = PathBuf::from("./tests/custom_config.toml");
        let cfg: Config = load_file(true, false, config_file).unwrap();
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

    #[test]
    #[serial]
    fn test_get_config_file_path() {
        let mut sentrypeer_c_config = unsafe { sentrypeer_config_new() };

        let sentrypeer_config = SentryPeerConfig {
            p: Box::into_raw(Box::new(unsafe { *sentrypeer_c_config })),
        };
        let config_file_path = get_config_file_path(sentrypeer_config).unwrap();

        eprintln!("test_get_config_file_path: config file is {config_file_path:?}");
        assert_ne!(config_file_path, PathBuf::from("./sentrypeer.toml"));

        unsafe { sentrypeer_config_destroy(&mut sentrypeer_c_config) };
    }
}
