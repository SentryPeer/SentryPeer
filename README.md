## Protect your SIP Servers from bad actors

<img alt="SentryPeer Logo" src="https://raw.githubusercontent.com/SentryPeer/SentryPeer/main/web-gui-theme/src/assets/logo.svg" width="100" height="100">

[![Stability: Active](https://masterminds.github.io/stability/active.svg)](https://masterminds.github.io/stability/active.html)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/sentrypeer/sentrypeer?sort=semver)](https://github.com/SentryPeer/SentryPeer/releases)
[![Docker Hub](https://img.shields.io/badge/docker-hub-brightgreen.svg)](https://hub.docker.com/r/sentrypeer/sentrypeer)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/23969/badge.svg)](https://scan.coverity.com/projects/sentrypeer-sentrypeer)
[![Build and Test](https://github.com/SentryPeer/SentryPeer/actions/workflows/main.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/main.yml)
[![CodeQL](https://github.com/SentryPeer/SentryPeer/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/codeql-analysis.yml)
[![Clang Static Analysis](https://github.com/SentryPeer/SentryPeer/actions/workflows/clang-analyzer.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/clang-analyzer.yml)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/5374/badge)](https://bestpractices.coreinfrastructure.org/projects/5374)

_Give us a star and follow us on [Twitter](https://twitter.com/sentrypeer)!_  

## Table of Contents
* [Introduction](#introduction)
* [Features](#features)
* [Talks](#talks)
* [Adoption](#adoption)
* [Design](#design)
* [Docker](#docker)
  * [Environment Variables](#environment-variables)
* [Installation](#installation)
  * [Homebrew (macOS or Linux)](#homebrew-macos-or-linux)
  * [Alpine Linux](#alpine-linux)
  * [Ubuntu Package](#ubuntu-package)
  * [Building from source](#building-from-source)
* [Running SentryPeer](#running-sentrypeer)
* [WebHook](#webhook)
* [RESTful API](#restful-api)
  * [Endpoint /health-check](#endpoint-health-check)
  * [Endpoint /ip-addresses](#endpoint-ip-addresses)
  * [Endpoint /ip-address/{ip-address}](#endpoint-ip-addressip-address)
  * [Endpoint /numbers/{phone-number}](#endpoint-numbersphone-number)
* [Syslog and Fail2ban](#syslog-and-fail2ban)
* [JSON Log Format](#json-log-format) 
* [Command Line Options](#command-line-options)
* [IPv6 Multicast Address](#ipv6-multicast-address)
* [License](#license)
* [Contributing](#contributing)
* [Project Website](#project-website)
* [Trademark](#trademark)
* [Questions, Bug reports, Feature Requests](#questions-bug-reports-feature-requests)
* [Special Thanks](#special-thanks)

### Introduction

SentryPeer<sup>&reg;</sup> is a fraud detection tool. It lets bad actors try to make phone calls and saves the IP address they came from and 
number they tried to call. Those details can then be used to raise notifications at the service providers network and the next 
time a user/customer tries to call a collected number, you can act anyway you see fit.

For example:

Let's say you are running your own VoIP PBX on site. What SentryPeer will allow you to do in this context, 
is dip into the list of phone numbers (using the RESTful API) when your users are making outbound calls. If you get a hit, 
you'll get a heads-up that potentially a device within your network is trying to call known probing phone numbers that 
have either been:

1. Numbers collected by SentryPeer nodes you are running yourself
2. Numbers seen by other SentryPeer nodes which have been replicated to your node via the peer to peer network

This would allow you to generate a notification from your monitoring systems before you rack 
up any expensive calls or something worse happens.

What would lead to this scenario?

1. Potential voicemail fraud. This can happen if you allow calling an
   inbound number (your DID/DDI) to get to your voicemail system, then
   prompt for a PIN. This PIN is weak and the voicemail system allows you
   to press '*' to call back the Caller ID that left a voicemail. The
   attacker has left a voicemail, and they then guess your PIN and call it
   back. The CLI is a known number that SentryPeer has seen. You can alert on it.
2. A device has been hijacked and/or a softphone or similar is using
   the credentials they stole off the phone's GUI and is trying to
   register to your system and make calls to a number seen by SentryPeer.
3. An innocent user is calling a phishing number or known expensive
   number etc. that SentryPeer has seen before.

Traditionally this data is shipped to a central place, so you don't own the data you've collected. This project is all about Peer to Peer sharing of that data. The user owning the data and various Service Provider / Network Provider related feeds of the data is the key bit for me. I'm sick of all the services out there that keep it and sell it. If you've collected it, you should have the choice to keep it and/or opt in to share it with other SentryPeer community members via p2p methods.

### Features

- [x] All code [Free/Libre and Open Source Software](https://www.gnu.org/philosophy/floss-and-foss.en.html)
- [x] FAST
- [x] User _owns their_ data
- [x] User can submit their own data if they want to (you need to enable p2p mode - `-p`) 
- [x] User gets other users' data **ONLY IF** they opt in to submit their data to the pool
- [x] Embedded Distributed Hash Table (DHT) node using [OpenDHT](https://github.com/savoirfairelinux/opendht/wiki/Running-a-node-in-your-program) (`-p` cli option)
- [x] Peer to Peer **sharing** of collected bad_actors using [OpenDHT](https://github.com/savoirfairelinux/opendht) (default off)
- [x] Peer to Peer data replication to **receive** collected bad_actors using [OpenDHT](https://github.com/savoirfairelinux/opendht) (default off)
- [x] Set your own DHT bootstrap node (`-b` cli option)
- [x] Multithreaded
- [x] UDP transport
- [x] TCP transport
- [ ] TLS transport
- [x] [JSON logging](#json-log-format) to a file
- [x] SIP mode can be disabled. This allows you to run SentryPeer in API mode or DHT mode only etc. i.e.
  not as a honeypot, but as a node in the SentryPeer community or to just serve replicated data
- [x] SIP responsive mode can be enabled to collect data - cli / env flag   
- [x] **Local** data copy for **fast access** - cli / env db location flag
- [x] **Local** API for **fast access** - cli / env flag
- [x] WebHook for POSTing bad actor json to a central location - cli / env flag
- [x] Integration with [SentryPeerHQ](https://sentrypeer.com) via OAuth2 bearer token
- [x] Query API for IP addresses of bad actors
- [ ] Query API for IPSET of bad actors
- [x] Query API for a particular IP address of a bad actor
- [x] Query API for attempted phone numbers called by bad actors
- [x] Query API for an attempted phone number called by a bad actor
- [x] [Fail2Ban](https://www.fail2ban.org/wiki/index.php/Main_Page) support via `syslog` as per [feature request](https://github.com/SentryPeer/SentryPeer/issues/6)
- [x] Local [sqlite](https://www.sqlite.org/index.html) database - feature / cli flag
- [ ] Analytics - opt in
- [ ] SDKs/libs for external access - [CGRateS](https://github.com/cgrates/cgrates) to start with or our own firewall with nftables
- [x] Small binary size for IoT usage
- [x] Cross-platform
- [ ] Firewall options to use distributed data in real time - [DHT](https://en.wikipedia.org/wiki/Distributed_hash_table)?
- [x] Container on [Docker Hub for latest build](https://hub.docker.com/r/sentrypeer/sentrypeer)
- [ ] BGP agent to peer with for blackholing collected IP addresses (similar to [Team Cymru Bogon Router Server Project](https://team-cymru.com/community-services/bogon-reference/bogon-reference-bgp/))
- [ ] SIP agent to return 404 or default destination for SIP redirects

### Talks

- TADSummit 2021 - https://blog.tadsummit.com/2021/11/17/sentrypeer/
- CommCon 2021 - https://2021.commcon.xyz/talks/sentrypeer-a-distributed-peer-to-peer-list-of-bad-ip-addresses-and-phone-numbers-collected-via-a-sip-honeypot
- ClueCon Weekly 2022 - https://youtu.be/DFxGHJI_0Wg
- UKNOF49 2022 ([presentation slides](https://indico.uknof.org.uk/event/59/contributions/801/attachments/1033/1520/UKNOF-49-2022-SentryPeer.pdf)) - https://indico.uknof.org.uk/event/59/contributions/801/

### Adoption

* [Kali Linux](https://pkg.kali.org/pkg/sentrypeer)
* Deutsche Telekom [T-Pot - The All In One Honeypot Platform](https://github.com/telekom-security/tpotce) [v22](https://github.com/telekom-security/tpotce/releases/tag/22.04.0) onwards 

![Matrix](https://img.shields.io/matrix/sentrypeer:matrix.org?label=matrix&logo=matrix)
[![slack](https://img.shields.io/badge/join-us%20on%20slack-gray.svg?longCache=true&logo=slack&colorB=brightgreen)](https://join.slack.com/t/sentrypeer/shared_invite/zt-zxsmfdo7-iE0odNT2XyKLP9pt0lgbcw)
[![SentryPeer on Twitter](https://img.shields.io/badge/follow-twitter-blue)](https://twitter.com/SentryPeer)

### Design

I started this because I wanted to do [C network programming](https://github.com/codeplea/Hands-On-Network-Programming-with-C) as all the projects I use daily are in C like [PostgreSQL](https://www.postgresql.org/), [OpenLDAP](https://www.openldap.org/), [FreeSWITCH](https://freeswitch.com/), [OpenSIPS](https://opensips.org/),
[Asterisk](https://www.asterisk.org/) etc. See
[Episode 414: Jens Gustedt on Modern C](https://www.se-radio.net/2020/06/episode-414-jens-gustedt-on-modern-c/) for why [C](https://en.wikipedia.org/wiki/C_(programming_language)) is a good choice.  For those interested, see my full podcast show list (https://www.se-radio.net/team/gavin-henry/) for [Software Engineering Radio](https://www.se-radio.net/)

### Docker

You can run the latest version of SentryPeer with [Docker](https://www.docker.com/). The latest version is available from [Docker Hub](https://hub.docker.com/r/sentrypeer/sentrypeer/).
Or build yourself:

    sudo docker build --no-cache -t sentrypeer .
    sudo docker run -d -p 5050:5060/tcp -p 5060:5060/udp -p 8082:8082 -p 4222:4222/udp sentrypeer:latest

Then you can check at `http://localhost:8082/ip-addresses` and `http://localhost:8082/health-check` to see if it's running.

#### Environment Variables

    ENV SENTRYPEER_DB_FILE=/my/location/sentrypeer.db
    ENV SENTRYPEER_API=1
    ENV SENTRYPEER_WEBHOOK_URL=https://my.webhook.url/events
    ENV SENTRYPEER_OAUTH2_CLIENT_ID=1234567890
    ENV SENTRYPEER_OAUTH2_CLIENT_SECRET=1234567890
    ENV SENTRYPEER_WEBHOOK=1
    ENV SENTRYPEER_SIP_RESPONSIVE=1
    ENV SENTRYPEER_SIP_DISABLE=1
    ENV SENTRYPEER_SYSLOG=1
    ENV SENTRYPEER_PEER_TO_PEER=1
    ENV SENTRYPEER_BOOTSTRAP_NODE=mybootstrapnode.com
    ENV SENTRYPEER_JSON_LOG=1
    ENV SENTRYPEER_JSON_LOG_FILE=/my/location/sentrypeer_json.log
    ENV SENTRYPEER_VERBOSE=1
    ENV SENTRYPEER_DEBUG=1

Either set these in the Dockerfile or in your `Dockerfile.env` file or docker run command.

Settings any of these to `0` will also _enable_ the feature. We _don't care_ what you set it to, just that it's set.

### Installation
 
Debian or Fedora packages are always available from the release page for the current version of SentryPeer:

   https://github.com/SentryPeer/SentryPeer/releases

#### Homebrew (macOS or Linux):

We have a [Homebrew Tap for this project](https://github.com/SentryPeer/homebrew-sentrypeer) (until we get more popular):

    brew tap sentrypeer/sentrypeer
    brew install sentrypeer

#### Alpine Linux:

SentryPeer is in [testing on Alpine Linux](https://gitlab.alpinelinux.org/alpine/aports/-/tree/master/testing/sentrypeer), so you can install it with the following command:

    apk -U add --no-cache -X https://dl-cdn.alpinelinux.org/alpine/edge/testing sentrypeer

#### Ubuntu Package

You can install SentryPeer from [our Ubuntu PPD](https://launchpad.net/~gavinhenry/+archive/ubuntu/sentrypeer) which
is currently for Ubuntu 20 LTS (Focal Fossa):

    sudo apt install software-properties-common
    sudo add-apt-repository ppa:gavinhenry/sentrypeer
    sudo apt-get update

This PPA can be added to your system manually by copying the lines below and adding them to your system's software 
sources:

    deb https://ppa.launchpadcontent.net/gavinhenry/sentrypeer/ubuntu focal main 
    deb-src https://ppa.launchpadcontent.net/gavinhenry/sentrypeer/ubuntu focal main

Then you can install SentryPeer:

    sudo apt-get install sentrypeer

#### Building from source

You have two options for installation from source. CMake or autotools. Autotools is recommended at the moment. A release is an autotools build.

If you are a Fedora user, you can install this via [Fedora copr](https://copr.fedorainfracloud.org/coprs/):

[https://copr.fedorainfracloud.org/coprs/ghenry/SentryPeer/](https://copr.fedorainfracloud.org/coprs/ghenry/SentryPeer/)

If you are going to build from this repository, you will need to have the following installed:

  - `git`, `autoconf`, `automake` and `autoconf-archive` (Debian/Ubuntu) 
  - `libosip2-dev` (Debian/Ubuntu) or `libosip2-devel` (Fedora)
  - `libsqlite3-dev` (Debian/Ubuntu) or `sqlite-devel` (Fedora)
  - `uuid-dev` (Debian/Ubuntu) or `libuuid-devel` (Fedora)
  - `libmicrohttpd-dev` (Debian/Ubuntu) or `libmicrohttpd-devel` (Fedora)
  - `libjansson-dev` (Debian/Ubuntu) or `jansson-devel` (Fedora)
  - `libpcre2-dev` (Debian/Ubuntu) or `pcre2-devel` (Fedora)
  - `libcurl-dev` (Debian/Ubuntu) or `libcurl-devel` (Fedora)
  - `libcmocka-dev` (Debian/Ubuntu) or `libcmocka-devel` (Fedora) - for unit tests

Debian/Ubuntu:

    sudo apt-get install git build-essential autoconf-archive autoconf automake libosip2-dev libsqlite3-dev \
    libcmocka-dev uuid-dev libcurl4-openssl-dev libpcre2-dev libjansson-dev libmicrohttpd-dev 

Fedora:

    sudo dnf install git autoconf automake autoconf-archive libosip2-devel libsqlite3-devel libcmocka-devel \
    libuuid-devel libmicrohttpd-devel jansson-devel libcurl-devel pcre2-devel

macOS:

    brew install git autoconf automake autoconf-archive libosip cmocka libmicrohttpd jansson curl pcre2

then (make check is highly recommended):

    ./bootstrap.sh
    ./configure
    make
    make check
    make install

### Running SentryPeer

Once built, you can run like so to start in **debug mode**, **respond** to SIP probes, enable the **RESTful API**, 
enable WebHooks and enable syslog logging ([use a package](https://github.com/SentryPeer/SentryPeer/releases) if you want [systemd](https://www.freedesktop.org/wiki/Software/systemd/)):

    ./sentrypeer -draps
    SentryPeer node id: e5ac3a88-3d52-4e84-b70c-b2ce83992d02
    Starting sentrypeer...
    API mode enabled, starting http daemon...
    SIP mode enabled...
    Peer to Peer DHT mode enabled...
    Starting peer to peer DHT mode using OpenDHT-C lib version '2.4.0'...
    Configuring local address...
    Creating sockets...
    Binding sockets to local address...
    Listening for incoming UDP connections...
    SIP responsive mode enabled. Will reply to SIP probes...
    Listening for incoming TCP connections...
    Peer to peer DHT mode started.
    DHT InfoHash for key 'bad_actors' is: 14d30143330e2e0e922ed4028a60ff96a59800ad
    Bootstrapping the DHT
    Waiting 5 seconds for bootstrapping to bootstrap.sentrypeer.org...
    Listening for changes to the bad_actors DHT key


when you get a probe request, you can see something like the following in the terminal:

```bash
Received (411 bytes): OPTIONS sip:100@XXX.XXX.XXX.XXX SIP/2.0
Via: SIP/2.0/UDP 91.223.3.152:5173;branch=z9hG4bK-515761064;rport
Content-Length: 0
From: "sipvicious"<sip:100@1.1.1.1>;tag=6434396633623535313363340131363131333837383137
Accept: application/sdp
User-Agent: friendly-scanner
To: "sipvicious"<sip:100@1.1.1.1>
Contact: sip:100@91.223.3.152:5173
CSeq: 1 OPTIONS
Call-ID: 679894155883566215079442
Max-Forwards: 70


read_packet_buf size is: 1024: 
read_packet_buf length is: 468: 
bytes_received size is: 411: 

Bad Actor is:
Event Timestamp: 2021-11-23 20:13:36.427515810
Event UUID: fac3fa20-8c2c-445b-8661-50a70fa9e873
SIP Message: OPTIONS sip:100@XXX.XXX.XXX.XXX SIP/2.0
Via: SIP/2.0/UDP 91.223.3.152:5173;branch=z9hG4bK-515761064;rport
From: "sipvicious" <sip:100@1.1.1.1>;tag=6434396633623535313363340131363131333837383137
To: "sipvicious" <sip:100@1.1.1.1>
Call-ID: 679894155883566215079442
CSeq: 1 OPTIONS
Contact: <sip:100@91.223.3.152:5173>
Accept: application/sdp
User-agent: friendly-scanner
Max-forwards: 70
Content-Length: 0


Source IP: 193.107.216.27
Called Number: 100
SIP Method: OPTIONS
Transport Type: UDP
User Agent: friendly-scanner
Collected Method: responsive
Created by Node Id: fac3fa20-8c2c-445b-8661-50a70fa9e873
SentryPeer db file location is: sentrypeer.db
Destination IP address of UDP packet is: xx.xx.xx.xx
```

You can see the data in the sqlite3 database called `sentrypeer.db` using [sqlitebrowser](https://sqlitebrowser.org/) or sqlite3 command line tool.

Here's a screenshot of the database opened using [sqlitebrowser](https://sqlitebrowser.org/) (it's big, so I'll just link to the image):

[sqlitebrowser exploring the sentrypeer.db](./screenshots/SentryPeer-sqlitebrowser.png)

### WebHook

There is a WebHook to POST a [JSON Log Format](#json-log-format) payload to [SentryPeerHQ](https://github.com/SentryPeer/SentryPeer_HQ) or
your own WebHook endpoint.  The WebHook is **not** enabled by default. You can configure the WebHook URL via `-w` or set 
the `SENTRYPEER_WEBHOOK_URL` env variable.

### RESTful API 

The RESTful API is complete for the current use cases. Please click the Watch button to be notified when more things come out :-)

#### Endpoint /health-check

Query the API to see if it's alive:

```bash
curl -v -H "Content-Type: application/json" http://localhost:8082/health-check

* Connected to localhost (127.0.0.1) port 8082 (#0)
> GET /health-check HTTP/1.1
> Host: localhost:8082
> User-Agent: curl/7.79.1
> Accept: */*
> Content-Type: application/json
> 
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Date: Mon, 24 Apr 2022 11:16:25 GMT
< Content-Type: application/json
< Access-Control-Allow-Origin: *
< X-Powered-By: SentryPeer
< X-SentryPeer-Version: 1.4.0
< Content-Length: 81
< 
{
  "status": "OK",
  "message": "Hello from SentryPeer!",
  "version": "1.0.0"
}
```

#### Endpoint /ip-addresses

List all the IP addresses that have been seen by SentryPeer:

```bash
curl -v -H "Content-Type: application/json" http://localhost:8082/ip-addresses

* Connected to localhost (127.0.0.1) port 8082 (#0)
> GET /ip-addresses HTTP/1.1
> Host: localhost:8082
> User-Agent: curl/7.79.1
> Accept: */*
> Content-Type: application/json
> 
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Date: Mon, 24 Jan 2022 11:17:05 GMT
< Content-Type: application/json
< Access-Control-Allow-Origin: *
< X-Powered-By: SentryPeer
< X-SentryPeer-Version: 1.0.0
< Content-Length: 50175
< 
{
  "ip_addresses_total": 396,
  "ip_addresses": [
    {
      "ip_address": "193.107.216.27",
      "seen_last": "2022-01-11 13:30:48.703603359",
      "seen_count":	"1263"
    },
    {
      "ip_address": "193.46.255.152"
      "seen_last": "2022-01-11 13:28:27.348926406",
      "seen_count": "3220"      
    }
    ...
  ]
}
```

#### Endpoint /ip-address/{ip-address}

Query a single IP address:

```bash
curl -v -H "Content-Type: application/json" http://localhost:8082/ip-address/8.8.8.8

* Connected to localhost (127.0.0.1) port 8082 (#0)
> GET /ip-addresses/8.8.8.8 HTTP/1.1
> Host: localhost:8082
> User-Agent: curl/7.79.1
> Accept: */*
> Content-Type: application/json
> 
* Mark bundle as not supporting multiuse
< HTTP/1.1 404 Not Found
< Date: Mon, 24 Jan 2022 11:17:57 GMT
< Content-Type: application/json
< Access-Control-Allow-Origin: *
< X-Powered-By: SentryPeer
< X-SentryPeer-Version: 1.0.0
< Content-Length: 33
< 
* Connection #0 to host localhost left intact
{
  "message": "No bad actor found"
}
```

#### Endpoint /numbers/{phone-number}

Query a phone number a bad actor tried to call with optional `+` prefix:

```bash
curl -v -H "Content-Type: application/json" http://localhost:8082/numbers/8784946812410967

* Connected to localhost (127.0.0.1) port 8082 (#0)
> GET /numbers/8784946812410967 HTTP/1.1
> Host: localhost:8082
> User-Agent: curl/7.79.1
> Accept: */*
> Content-Type: application/json
> 
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Date: Mon, 24 Jan 2022 11:19:53 GMT
< Content-Type: application/json
< Access-Control-Allow-Origin: *
< X-Powered-By: SentryPeer
< X-SentryPeer-Version: 1.0.0
< Content-Length: 46
< 
{
  "phone_number_found": "8784946812410967"
}
```

### Syslog and Fail2ban

With `sentrypeer -s`, you parse syslog and use Fail2Ban to block the IP address of the bad actor:

```syslog
Nov 30 21:32:16 localhost.localdomain sentrypeer[303741]: Source IP: 144.21.55.36, Method: OPTIONS, Agent: sipsak 0.9.7
```

### JSON Log Format 

With `sentrypeer -j`, you can produce a JSON log file of the bad actor's IP address and the phone number they tried to call 
plus other metadata (set a custom log file location with `-l`):

```json
{
   "app_name":"sentrypeer",
   "app_version":"v1.4.0",
   "event_timestamp":"2022-02-22 11:19:15.848934346",
   "event_uuid":"4503cc92-26cb-4b3e-bb33-69a83fa09321",
   "created_by_node_id":"4503cc92-26cb-4b3e-bb33-69a83fa09321",
   "collected_method":"responsive",
   "transport_type":"UDP",
   "source_ip":"45.134.144.128",
   "destination_ip":"XX.XX.XX.XX",
   "called_number":"0046812118532",
   "sip_method":"OPTIONS",
   "sip_user_agent":"friendly-scanner",
   "sip_message":"full SIP message"
}
```

### Command Line Options

```bash
./sentrypeer -h
Usage: sentrypeer [-h] [-V] [-w] [-j] [-p] [-b bootstrap.example.com] [-f fullpath for sentrypeer.db] [-l fullpath for sentrypeer_json.log] [-r] [-R] [-a] [-s] [-v] [-d]

Options:
  -h,      Print this help
  -V,      Print version
  -f,      Set 'sentrypeer.db' location or use SENTRYPEER_DB_FILE env
  -j,      Enable json logging or use SENTRYPEER_JSON_LOG env
  -p,      Enable Peer to Peer mode or use SENTRYPEER_PEER_TO_PEER env
  -b,      Set Peer to Peer bootstrap node or use SENTRYPEER_BOOTSTRAP_NODE env
  -a,      Enable RESTful API mode or use SENTRYPEER_API env
  -w,      Set WebHook URL for bad actor json POSTs or use SENTRYPEER_WEBHOOK_URL env
  -r,      Enable SIP responsive mode or use SENTRYPEER_SIP_RESPONSIVE env
  -R,      Disable SIP mode completely or use SENTRYPEER_SIP_DISABLE env
  -l,      Set 'sentrypeer_json.log' location or use SENTRYPEER_JSON_LOG_FILE env
  -s,      Enable syslog logging or use SENTRYPEER_SYSLOG env
  -v,      Enable verbose logging or use SENTRYPEER_VERBOSE env
  -d,      Enable debug mode or use SENTRYPEER_DEBUG env

Report bugs to https://github.com/SentryPeer/SentryPeer/issues

See https://sentrypeer.org for more information.
```

### IPv6 Multicast Address

The project has an IANA IPv6 multicast address for the purpose of sending messages between SentryPeer peers.

    Addresses: FF0X:0:0:0:0:0:0:172
    Description: SentryPeer
    Contact: Gavin Henry <ghenry at sentrypeer.org>
    Registration Date: 2022-01-26

Please see http://www.iana.org/assignments/ipv6-multicast-addresses

The assigned variable-scope address -- which can also be listed as "FF0X::172" for short -- the "X" denotes any possible scope.

### License
 
Great reading - [How to choose a license for your own work](https://www.gnu.org/licenses/license-recommendations.en.html)

This work is dual-licensed under GPL 2.0 and GPL 3.0.

`SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only`

### Contributing

See [CONTRIBUTING](./CONTRIBUTING.md)

### Project Website

https://sentrypeer.org

### Trademark

[**SENTRYPEER** is a registered trademark](https://trademarks.ipo.gov.uk/ipo-tmcase/page/Results/1/UK00003700947) of Gavin Henry

### Questions, Bug reports, Feature Requests

New issues can be raised at:

https://github.com/SentryPeer/SentryPeer/issues

It's okay to raise an issue to ask a question.

### Special Thanks

Special thanks to:
  - [psanders](https://github.com/psanders) from the [Routr](https://github.com/fonoster/routr) project for [tips on re-working this README.md](https://mobile.twitter.com/pedrosanders_/status/1554572884714070019) file.
  - [Fly.io](https://fly.io) for crediting the SentryPeer account for hosting the [SentryPeer HQ web app](https://sentrypeer.com) on their infrastructure
  - [AppSignal](https://www.appsignal.com/) for Application performance monitoring sponsorship in the [SentryPeer HQ web app](https://sentrypeer.com)
  - [David Miller](http://davidmiller.io/) for the design of the SentryPeer [Web GUI theme](./web-gui-theme) and [logo](./web-gui-theme/src/assets/logo.svg). Very kind of you!
  - [@garymiller](https://github.com/garyemiller) for the feature request of syslog and Fail2ban as per [ Fail2ban Integration via syslog #6](https://github.com/SentryPeer/SentryPeer/issues/6) 
  - [@joejag](https://github.com/joejag) for the [Pull Request](https://github.com/SentryPeer/SentryPeer/pull/19) for the start of [Terraform recipes to launch SentryPeer on different cloud providers #12](https://github.com/SentryPeer/SentryPeer/issues/12)

