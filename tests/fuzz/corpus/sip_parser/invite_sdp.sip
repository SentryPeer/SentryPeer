INVITE sip:100@example.com SIP/2.0
v: SIP/2.0/TCP 10.0.0.1:5060;branch=z9hG4bK6
f: <sip:1000@example.com>;tag=6
t: <sip:100@example.com>
i: pqr@10.0.0.1
CSeq: 1 INVITE
Route: <sip:proxy.example.com;lr>
Record-Route: <sip:proxy.example.com;lr>
c: application/sdp
l: 60

v=0
o=- 1 1 IN IP4 10.0.0.1
s=-
c=IN IP4 10.0.0.1
t=0 0
m=audio 4000 RTP/AVP 0
a=rtpmap:0 PCMU/8000
