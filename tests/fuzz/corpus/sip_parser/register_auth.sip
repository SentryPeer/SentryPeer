REGISTER sip:example.com SIP/2.0
Via: SIP/2.0/UDP 10.0.0.1:5060;branch=z9hG4bK5;rport
From: "bob" <sip:1000@example.com>;tag=5
To: <sip:1000@example.com>
Call-ID: mno@10.0.0.1
CSeq: 2 REGISTER
Contact: <sip:1000@10.0.0.1:5060;transport=udp>;expires=60
Authorization: Digest username="1000", realm="example.com", nonce="abc", uri="sip:example.com", response="0123456789abcdef0123456789abcdef", algorithm=MD5, qop=auth, cnonce="x", nc=00000001
Allow: INVITE, ACK, BYE, CANCEL, OPTIONS
Content-Length: 0

