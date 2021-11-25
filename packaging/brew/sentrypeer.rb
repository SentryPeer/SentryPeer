class Sentrypeer < Formula
  desc "SIP honeypot for a distributed p2p list of bad IP addresses and phone numbers"
  homepage "https://sentrypeer.org"
  url "https://github.com/SentryPeer/SentryPeer/releases/download/v0.0.2/sentrypeer-0.0.2.tar.gz"
  sha256 "d7c80e0779504a3e8b48712a863f85bc076e8ef175762e2c0904bc6a268258a2"
  license any_of: ["GPL-2.0-only", "GPL-3.0-only"]

  depends_on "cmocka" => [:build, :test]
  depends_on "libosip"

  def install
    system "./configure", *std_configure_args, "--disable-silent-rules"
    system "make", "install"
  end

  test do
    system "sentrypeer", "-V"
  end
end
