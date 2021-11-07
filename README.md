## SentryPeer

<img alt="SentryPeer Logo" src="https://raw.githubusercontent.com/SentryPeer/SentryPeer/main/web-gui-theme/src/assets/logo.svg" width="100" height="100"> 

A distributed list of bad IP addresses and phone numbers collected via a SIP Honeypot.

[![Coverity Scan Build Status](https://scan.coverity.com/projects/23969/badge.svg)](https://scan.coverity.com/projects/sentrypeer-sentrypeer)
[![build_and_test](https://github.com/SentryPeer/SentryPeer/actions/workflows/main.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/main.yml)
[![CodeQL](https://github.com/SentryPeer/SentryPeer/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/codeql-analysis.yml)
[![Clang Static Analysis](https://github.com/SentryPeer/SentryPeer/actions/workflows/clang-analyzer.yml/badge.svg)](https://github.com/SentryPeer/SentryPeer/actions/workflows/clang-analyzer.yml)

## Introduction

This is basically a fraud detection tool. It lets bad actors try to make phone calls and saves the IP address they came from and number they tried to call. Those details are then used to block them at the service providers network and the next time a user/customer tries to call a collected number, it's blocked.

Traditionally this data is shipped to a central place, so you don't own the data you've collected. This project is all about Peer to Peer sharing of that data. The user owning the data and various Service Provider / Network Provider related feeds of the data is the key bit for me. I'm sick of all the services out there that keep it and sell it. If you've collected it, you should have the choice to keep it and/or opt in to share it with other SentryPeer community members via p2p methods.

Of course, if you don't want to run any of this and just buy access to the data that users have opted in to share, then that's a choice too. One day, maybe.

The sharing part...you only get other users' data if you [share yours](https://en.wikipedia.org/wiki/Tit_for_tat#Peer-to-peer_file_sharing). That's the key. It could be used (the sharing of data logic/feature) in many projects too if I get it right :-)

## Screenshots

Here's a mock up of the web UI which is subject to change.

[![SentryPeer Web GUI mock up](./screenshots/SentryPeer-Web-GUI-screenshot.png)](./screenshots/SentryPeer-Web-GUI-screenshot.png)

Screenshots of agents and APIs to come...

## Goals

- [x] All code [Free/Libre and Open Source Software](https://www.gnu.org/philosophy/floss-and-foss.en.html)
- [ ] FAST
- [x] User _owns their_ data
- [ ] User can submit their own data if they want to - _opt out_ (default is to submit data)
- [ ] User gets other users' data ([Tit for tat?](https://en.wikipedia.org/wiki/Tit_for_tat#Peer-to-peer_file_sharing)) **ONLY IF** they opt in to submit their data to the pool ([DHT](https://en.wikipedia.org/wiki/Distributed_hash_table)? - need to do a [PoC](https://en.wikipedia.org/wiki/Proof_of_concept))
- [ ] User can **pay to get all data collected** via [SentryPeer commercial website](https://sentrypeer.com) (one day, maybe.)
- [ ] Data is max 7(?) days old as useless otherwise
- [ ] **Local** data copy for **fast access** - feature / cli flag
- [ ] **Local** API for **fast access** - feature / cli flag
- [ ] **Local** Web GUI for **fast access** - feature / cli flag
- [ ] Peer to Peer data replication - feature / cli flag
- [ ] Local [sqlite](https://www.sqlite.org/index.html)/[lmdb](https://www.symas.com/symas-embedded-database-lmdb) database - feature / cli flag
- [ ] Analytics - opt in
- [ ] SDKs/libs for external access - [CGRateS](https://github.com/cgrates/cgrates) to start with and maybe [Fail2Ban](https://www.fail2ban.org/wiki/index.php/Main_Page) or our own with nftables
- [ ] Small binary size for IoT usage
- [ ] Cross-platform
- [ ] Firewall options to use distributed data in real time - [DHT](https://en.wikipedia.org/wiki/Distributed_hash_table)?
- [ ] Container on Docker Hub for latest build - Reference https://github.com/natm/iocontrollergw/blob/master/.github/workflows/cd.yaml and https://github.com/natm/iocontrollergw/blob/master/.github/workflows/ci.yaml (plus Nat's Dockerfile :-) )
- [ ] BGP agent to peer with for blackholing collected IP addresses (similar to [Team Cymru Bogon Router Server Project](https://team-cymru.com/community-services/bogon-reference/bogon-reference-bgp/))
- [ ] SIP agent to return 404 or default destination for SIP redirects

## Design

TBD :-)

I started this because I wanted to do [C network programming](https://github.com/codeplea/Hands-On-Network-Programming-with-C) as all the projects I use daily are in C like [PostgreSQL](https://www.postgresql.org/), [OpenLDAP](https://www.openldap.org/), [FreeSWITCH](https://freeswitch.com/), [OpenSIPS](https://opensips.org/),
[Asterisk](https://www.asterisk.org/) etc. See
[Episode 414: Jens Gustedt on Modern C](https://www.se-radio.net/2020/06/episode-414-jens-gustedt-on-modern-c/) for why [C](https://en.wikipedia.org/wiki/C_(programming_language)) is a good choice.  For those interested, see my full podcast show list (https://www.se-radio.net/team/gavin-henry/) for [Software Engineering Radio](https://www.se-radio.net/)

### Installation
 
See [prototype](./prototype) folder for now.

### License
 
Great reading - [How to choose a license for your own work](https://www.gnu.org/licenses/license-recommendations.en.html)

This work is dual-licensed under GPL 2.0 and GPL 3.0.

`SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only`

### Contributing

See [CONTRIBUTING](./CONTRIBUTING.md)

### Project Website

https://sentrypeer.org
 
### Commercial Services (one day, maybe)
 
https://sentrypeer.com

### Trademark

[**SENTRYPEER** is a pending registered trademark](https://trademarks.ipo.gov.uk/ipo-tmcase/page/Results/1/UK00003700947) of Gavin Henry

### Questions, Bug reports, Feature Requests

New issues can be raised at:

https://github.com/ghenry/SentryPeer/issues

It's okay to raise an issue to ask a question.

### Special Thanks

Special thanks to [David Miller](http://davidmiller.io/) for the design of the SentryPeer [Web GUI theme](./web-gui-theme) and [logo](./web-gui-theme/src/assets/logo.svg). Very kind of you!

