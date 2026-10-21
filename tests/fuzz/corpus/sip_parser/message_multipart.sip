MESSAGE sip:100@example.com SIP/2.0
Via: SIP/2.0/UDP 10.0.0.1:5060;branch=z9hG4bK8
From: <sip:1000@example.com>;tag=9
To: <sip:100@example.com>
Call-ID: vwx@10.0.0.1
CSeq: 1 MESSAGE
Content-Type: multipart/mixed;boundary=b
Content-Length: 70

--b
Content-Type: text/plain

hello
--b
Content-Type: application/sdp

v=0
--b--
