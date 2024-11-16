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
        .stdout(predicate::str::contains("sentrypeer 4.0.0"));
}
