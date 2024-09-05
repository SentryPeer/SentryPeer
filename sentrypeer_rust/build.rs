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
use cbindgen;
use std::env;
use std::path::PathBuf;

fn main() {
    // Deal with the SentryPeer C library
    //
    // Tell cargo to tell rustc to link the sentrypeer
    // shared library and how to find it
    println!("cargo:rustc-link-search=../.libs"); // Autotools
    println!("cargo:rustc-link-search=../build"); // CMake
    println!("cargo:rustc-link-lib=sentrypeer");

    // Our other SentryPeer dependencies
    println!("cargo:rustc-link-lib=jansson");
    println!("cargo:rustc-link-lib=uuid");
    println!("cargo:rustc-link-lib=curl");
    println!("cargo:rustc-link-lib=sqlite3");
    println!("cargo:rustc-link-lib=osipparser2");

    // Check to see if OpenDHT-C is wanted in config.h and is not != 0 - only works via
    // autotools at the moment. Need to figure out CMake vars.
    let opendht = std::fs::read_to_string("../config.h").unwrap();
    if opendht.contains("#define HAVE_OPENDHT_C 1") {
        println!("cargo:rustc-link-lib=opendht-c");
    }

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("wrapper.h")
        // Pick the functions we want to generate bindings for
        .allowlist_function("sentrypeer_config_new|sentrypeer_config_destroy")
        .allowlist_function("sip_message_event_new|sip_message_event_destroy")
        .allowlist_function("sip_log_event")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings.rs, so can't use SentryPeer C lib!");

    // Generate our C header file for our Rust code
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    cbindgen::generate(crate_dir)
        .expect("Unable to generate our cbindings, so no C header file")
        .write_to_file("../src/sentrypeer_rust.h");
}
