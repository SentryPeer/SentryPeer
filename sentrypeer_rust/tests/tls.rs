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
#[ignore = "not yet implemented"]
#[allow(clippy::zombie_processes)]
fn tls_probe() {
    // Delete our JSON log file
    let mut json_log = Command::new("rm");
    json_log
        .args(["-f", "./sentrypeer_json.log"])
        .assert()
        .success();

    // Start sentrypeer in the background
    let mut cmd = std::process::Command::new("../sentrypeer");
    // Enable debug mode, responsive SIP mode, our certs and JSON logging using the
    // default JSON log file
    cmd.arg("-drj");
    cmd.args(["-t", "../tests/unit_tests/127.0.0.1.pem"]);
    cmd.args(["-k", "../tests/unit_tests/127.0.0.1-key.pem"]);
    cmd.args(["-z", "127.0.0.1:5061"]);

    let mut sentrypeer = cmd.spawn().expect("Failed to start sentrypeer");

    // Give sentrypeer time to start
    std::thread::sleep(std::time::Duration::from_secs(1));

    // Run our SIP tester
    let mut sipexer_cmd = Command::new("../tests/tools/sipexer");
    sipexer_cmd.arg("-tls-insecure").arg("tls:127.0.0.1:5061");
    sipexer_cmd.assert().failure();

    // Check that the JSON log file has been created
    let mut json_log = Command::new("cat");
    json_log.arg("../sentrypeer_json.log");
    json_log
        .assert()
        .success()
        .stdout(predicate::str::contains("TLS"));

    // Kill sentrypeer
    let _ = sentrypeer.kill();

    // Delete our JSON log file again
    let mut json_log = Command::new("rm");
    json_log
        .args(["-f", "./sentrypeer_json.log"])
        .assert()
        .success();

    // Delete our sentrypeer.db file
    let mut db = Command::new("rm");
    db.args(["-f", "./sentrypeer.db"]).assert().success();
}
