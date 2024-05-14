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

// https://github.com/rustls/rustls/blob/main/examples/src/bin/simpleserver.rs
use std::error::Error as StdError;
use std::fs::File;
use std::io::{BufReader, Read, Write};
use std::net::TcpListener;
use std::sync::Arc;

// #[no_mangle]
// pub extern "C" fn listen_tls() -> Result<(), Box<dyn StdError>> {
pub fn listen_tls() -> Result<(), Box<dyn StdError>> {

    let cert_file = "../tests/tools/127.0.0.1.pem";
    let private_key_file = "../tests/tools/127.0.0.1-key.pem";

    let certs = rustls_pemfile::certs(&mut BufReader::new(&mut File::open(cert_file)?))
        .collect::<Result<Vec<_>, _>>()?;
    let private_key =
        rustls_pemfile::private_key(&mut BufReader::new(&mut File::open(private_key_file)?))?
            .unwrap();
    let config = rustls::ServerConfig::builder()
        .with_no_client_auth()
        .with_single_cert(certs, private_key)?;

    let listener = TcpListener::bind(format!("[::]:{}", 4443)).unwrap();
    let (mut stream, _) = listener.accept()?;

    let mut conn = rustls::ServerConnection::new(Arc::new(config))?;
    conn.complete_io(&mut stream)?;

    conn.writer()
        .write_all(b"Hello from the server")?;
    conn.complete_io(&mut stream)?;
    let mut buf = [0; 64];
    let len = conn.reader().read(&mut buf)?;
    println!("Received message from client: {:?}", &buf[..len]);

    Ok(())
}

/// The simplest function used to confirm that calling our Rust library from C is working
#[no_mangle]
pub extern "C" fn display_rust() {
    println!("Greetings from Rust");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        display_rust();
    }
    
    // #[test]
    // fn test_listen_tls() {
    //     listen_tls().unwrap();
    // }
}
