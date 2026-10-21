SIP/2.0 407 Proxy Authentication Required
Via: SIP/2.0/UDP 10.0.0.1:5060;branch=z9hG4bK7
To: <sip:100@example.com>;tag=7
From: <sip:1000@example.com>;tag=8
Call-ID: stu@10.0.0.1
CSeq: 1 INVITE
Proxy-Authenticate: Digest realm="example.com", nonce="abc", opaque="def", algorithm=MD5, qop="auth,auth-int"
Content-Length: 0

