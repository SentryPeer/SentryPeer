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
use assert_cmd::Command;
use predicates::prelude::*;

// Handle finding autotools and CMake builds of sentrypeer binary
fn get_cmd() -> Command {
    if let Ok(bin) = std::env::var("SENTRYPEER_BIN") {
        Command::new(bin)
    } else if std::path::Path::new("../sentrypeer").exists() {
        Command::new("../sentrypeer")
    } else if std::path::Path::new("../build/sentrypeer").exists() {
        Command::new("../build/sentrypeer")
    } else {
        Command::new("sentrypeer")
    }
}

#[test]
fn help_shown_on_unknown_args() {
    let mut cmd = get_cmd();
    cmd.arg("--does-not-exist");

    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("Usage"));
}

#[test]
fn check_about() {
    let mut cmd = get_cmd();
    cmd.arg("-h");

    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Protect your SIP Servers"));
}

#[test]
fn check_version() {
    let mut cmd = get_cmd();
    cmd.arg("-V");

    cmd.assert()
        .success()
        .stdout(predicate::str::contains("sentrypeer 5"));
}
