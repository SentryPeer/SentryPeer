use assert_cmd::Command;
use predicates::prelude::*;

#[test]
#[ignore = "not yet implemented"]
fn tls_probe() {
    let mut cmd = Command::new("../sentrypeer");
    // Enable debug mode, responsive SIP mode, and JSON logging using the default JSON
    // log file
    cmd.arg("-drj");

    // Debug mode will show the transport type which is TLS
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("TLS"));

    let mut sipexer_cmd = Command::new("../tests/tools/sipexer");
    sipexer_cmd.arg("-tls-insecure").arg("tls:127.0.0.1:5061");
    sipexer_cmd.assert().success();
}
