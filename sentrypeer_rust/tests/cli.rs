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
use assert_cmd::Command;
use predicates::prelude::*;

#[test]
fn help_shown_on_unknown_args() {
    let mut cmd = Command::new("../sentrypeer");
    cmd.arg("--does-not-exist");

    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("Usage"));
}

#[test]
fn check_about() {
    let mut cmd = Command::new("../sentrypeer");
    cmd.arg("-h");

    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Protect your SIP Servers"));
}

#[test]
fn check_version() {
    let mut cmd = Command::new("../sentrypeer");
    cmd.arg("-V");

    cmd.assert()
        .success()
        .stdout(predicate::str::contains("sentrypeer 4.0.3"));
}
