# SentryPeer

A distributed list of bad IP addresses and phone numbers collected via a SIP Honeypot.

## Introduction

This is basically a fraud detection tool. It lets bad actors try to make phone calls and saves the IP address they came from and number they tried to call. Those details are then used to block them at the service providers network and the next time a user/customer tries to call a collected number, it's blocked.

Traditionally this data is shipped to a central place, so you don't own the data you've collected. This project is all about Peer to Peer sharing of that data. The user owning the data and various Service Provider / Network Provider related feeds of the data is the key bit for me. I'm sick of all the services out there that keep it and sell it. If you've collected it, you should have the choice to keep it and/or opt in to share it with other SentryPeer community members via p2p methods.

Of course, if you don't want to run any of this and just buy access to the data that users have opted in to share, then that's a choice too. One day, maybe.

The sharing part...you only get other users' data if you share yours. That's the key. It could be used (the sharing of data logic/feature) in many projects too if I get it right :-)


## Goals

- [x] All code [Free/Libre and Open Source Software](https://www.gnu.org/philosophy/floss-and-foss.en.html)
- [ ] FAST
- [x] User _owns their_ data
- [ ] User can submit their own data if they want to - _opt out_ (default is to submit data)
- [ ] User gets other users' data **ONLY IF** they opt in to submit their data to the pool ([DHT](https://en.wikipedia.org/wiki/Distributed_hash_table)? - need to do a [PoC](https://en.wikipedia.org/wiki/Proof_of_concept))
- [ ] User can **pay to get all data collected** via [SentyPeer commercial website](https://sentrypeer.com) (one day, maybe..)
- [ ] Data is max 7(?) days old as useless otherwise
- [ ] **Local** data copy for **fast access** - feature / cli flag
- [ ] **Local** API for **fast access** - feature / cli flag
- [ ] **Local** Web GUI for **fast access** - feature / cli flag
- [ ] Peer to Peer data replication - feature / cli flag
- [ ] Local [sqlite](https://www.sqlite.org/index.html)/[lmdb](https://www.symas.com/symas-embedded-database-lmdb) database - feature / cli flag
- [ ] Analytics - opt in
- [ ] SDKs/libs for external access - [CGRateS](https://github.com/cgrates/cgrates) to start with and maybe [Fail2Ban](https://www.fail2ban.org/wiki/index.php/Main_Page) or our own with nftables
- [ ] Small binary size for IoT usage
- [ ] Cross platform
- [ ] Firewall options to use distributed data in real time - [DHT](https://en.wikipedia.org/wiki/Distributed_hash_table)?
- [ ] Container on Docker Hub for latest build - Reference https://github.com/natm/iocontrollergw/blob/master/.github/workflows/cd.yaml and https://github.com/natm/iocontrollergw/blob/master/.github/workflows/ci.yaml (plus Nat's Dockerfile :-) )
- [ ] BGP agent to peer with for blackholing collected IP addresses (similar to [Team Cymru Bogon Router Server Project](https://team-cymru.com/community-services/bogon-reference/bogon-reference-bgp/))
- [ ] SIP agent to return 404 or default destination for SIP redirects

## Design

TBD :-)

Final language choice will probably be [C](https://en.wikipedia.org/wiki/C_(programming_language)), but looking at [Rust](https://www.rust-lang.org/), [Go](https://golang.org/) and [Elixir](https://elixir-lang.org/). Probably C with [Perl](https://www.perl.org/) for tests. I started this because
I wanted to do C network programming as all the projects I use daily are in C like [PostgreSQL](https://www.postgresql.org/), [OpenLDAP](https://www.openldap.org/), [FreeSWITCH](https://freeswitch.com/), [OpenSIPS](https://opensips.org/),
[Asterisk](https://www.asterisk.org/) etc. But then there's a LOT of Go and I'm doing a podcast on Rust in Nov 2021. Just done one on Dart. See my podcast
show list (https://www.se-radio.net/team/gavin-henry/) for [Software Engineering Radio](https://www.se-radio.net/)

### Installation
 
See [prototype](./prototype) folder for now.

### License
 
Great reading - [How to choose a license for your own work](https://www.gnu.org/licenses/license-recommendations.en.html)

This work is dual-licensed under GPL 2.0 and GPL 3.0.

`SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only`

### Project Website

https://sentrypeer.org
 
### Commercial Services (one day, maybe)
 
https://sentrypeer.com

