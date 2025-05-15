# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Changes
- Replace `jansson` with `serde_json` for JSON parsing/generation and switch to using Rust versions
- Replace `libcurl` with `reqwest` for HTTP requests and switch to using Rust versions

## [4.0.4] - 2025-03-17

### Fixes
- Switch to `Args::parse_from()`, so cli works when using [musl libc](https://www.musl-libc.org/)
as per [FFI, ALpine, musl and C](https://users.rust-lang.org/t/binary-wont-run-ffi-alpine-musl-and-c/127012/13)
and [aports issue](https://gitlab.alpinelinux.org/alpine/aports/-/merge_requests/81324)

## [4.0.3] - 2025-02-16

### Fixes
- Fix build issues on Alpine Linux due to [ring not building](https://github.com/briansmith/ring/issues/2378)

## [4.0.2] - 2025-02-15

### Packaging
- Maintenance release for `make dist`.

## [4.0.1] - 2025-02-13

### Changes
- UDP socket now created using `socket2` crate to set `set_reuse_address()` and
`set_nonblocking()` options e.g. `SO_REUSEADDR`.
- Clean up SIP reply from RUST side to have correct CRLF

## [4.0.0] - 2024-12-19

### Changes
- SIP TLS support, using [Rust](https://www.rust-lang.org/) and [Tokio-rustls](https://github.com/rustls/tokio-rustls) 
- If no `cert.pem` or `key.pem` files are found, we automatically create them for you in the current working directory for a seemless start up experience
- `-N` option to revert to old C way with `select()` for TCP and UDP
- TCP, UDP now all powered by Rust and Tokio
- We now support TLS configuration loaded from config using [confy](https://crates.io/crates/confy)
- If a cert or key file is not found using `-t` or `-k` options, SentryPeer will 
  ask you if you'd like to create them - we use the very nice [rcgen](https://crates.io/crates/rcgen) for this
- New `-z`, -t` and `-k` options for SIP TLS support
- Command Line Interface (CLI) now powered by Rust using [clap-rs](https://github.com/clap-rs/clap)
- Environment variables `SENTRYPEER_CERT`, `SENTRYPEER_KEY` and `SENTRYPEER_TLS_LISTEN_ADDRESS`
- We now look for `cargo` and `rustc`, as we are porting parts of SentryPeer to Rust. New features will be written in Rust
- SIP TLS support and other Rust-powered features will not be built if Rust is not detected. 
- `make check` now runs Rust tests
- `make clean` cleans up Rust builds
- `--enable-debug` argument added to `./configure` to enable debug mode for Rust code. Default is `--release`
- `--enable-silent-rules` and `make V=1` honoured in Rust code build
- `--disable-rust` argument added to `./configure` so you can disable Rust code build
- `CMakeLists.txt` updated to build Rust code and run all tests. We're using https://github.com/corrosion-rs/corrosion for this
- README.md updated to reflect the new Rust code build options, and CMake build instructions that now also 
  include Rust code and running our tests
- Show `CPPFLAGS` and `LDFLAGS` in `configure` status summary as needed to do macOS builds if on arm64  

## [3.0.2] - 2023-11-17

### Fixes
- Check `user_agent_header->hvalue` is not NULL as you can get a blank user-agent value in malformed SIP packets

## [3.0.1] - 2023-07-19
- Add a check for a response code of 403 to trigger a new access token request for WebHooks in addition to a 401

### Changes
- `-pedantic-errors` is now enabled by default in `Makefile.am`

### Fixes
- Switch to `getsockname()` to correctly get destination IP address for SIP TCP mode
- Check for a blank `User-agent` SIP header and add a test case. Only a missing one was catered for before. 

## [3.0.0] - 2023-04-26
- OAuth2 support for sending events to the [SentryPeerHQ](https://sentrypeer.com) RESTful API (client_credentials grant type) with
  a Bearer token in the Authorization header

### Fixes
- Clean up memory leaks in libcurl code when sending events to a WebHook url
- Fix segfault on parsing part of a 'To' SIP header that could be NULL
- Fix failing sqlite test on macos 

## [2.0.1] - 2023-02-16
- Re-work dht memory usage
- cppcheck work in tests
- Clearly state that Peer to Peer mode is off by default in the docs and that setting an environment variable to 
  anything, e.g. `0` will enable that particular mode. We _don't care_ what you set it to, just that it's set.

### Fixes
- Fix config trying to free an OpenDHT member that isn't present if not built with OpenDHT

## [2.0.0] - 2022-09-01
- Removed local incomplete web gui. This will be replaced by a new FL/OSS project called [SentryPeerHQ](https://github.com/SentryPeer/SentryPeer_HQ)
- Added optional `-w https://my_sentrypeerhq_onprem_or_cloud_url/events` WebHook cli/env var to set URL for bad actor 
  JSON to be sent to SentryPeerHQ for the dashboard and other services

### Breaking Changes
- `-w` now requires a URL to be set as it is used for the WebHook to [SentryPeerHQ](https://github.com/SentryPeer/SentryPeer_HQ). 
  Before it enabled the local Web UI which has been removed.

### Tests
- Added tests for new WebHook feature

## [1.4.1] - 2022-05-08
- SIP TCP mode first version
- New `-b` flag to set your own DHT bootstrap node (default: `bootstrap.sentrypeer.org:4222`)
- Error handing for `-p` flag if OpenDHT support is not compiled in
- Debug mode now shows what OpenDHT-C library version we're using
- Debug mode for p2p now shows the correct `event_uuid` when checking for duplicate events
- Coverity scan fixes

### Packaging
- Remove `-w` flag from deb and rpm packaging defaults

### Tests
- New tests for `sip_message_event` type and related functions

## [1.4.0] - 2022-03-29
- Peer to Peer mode with CLI (`-p`) and ENV options to enable, added for DHT node using the OpenDHT library
- Added `-R` flag to completely disable SIP mode. You can then run SentryPeer in API mode or DHT mode only etc. i.e. 
  not as a honeypot, but to serve local data
- Added Peer to Peer bad_actor sharing (data replication), e.g. it gets saved on the DHT and consumed by other peers
- RESTful API numbers resource (`/numbers`) now only returns numbers like +441234567890 or 441234567890
- RESTful API number resource (`/numbers/{number}`) now only accepts +441234567890 or 441234567890 formats
- Generate a SentryPeer node ID at startup to be used in all `bad_actor` events
- Stop the SIP daemon thread correctly on shutdown
- If OpenDHT-C is detected, check it is at least version 2.3.5. If Peer to Peer mode is not needed, you can 
  use `--disable-opendht`
- Remove the [Zyre](https://github.com/zeromq/zyre) library dependency as it is not needed anymore

### Breaking changes
- `--disable-zyre` option has now been removed, so `./configure` will now fail if you try to build with that option

### Tests
- More configuration CLI options tested
- Bad Actor to JSON conversion tests - used to save on the DHT
- Memory cleanups on tests
- UUID parsing tests
- JSON parsing tests
- Additional SQLite3 database select tests
- `bad_actor_exists` tests

## [1.2.0] - 2022-02-25
- Add signal handlers to gracefully shut down the SIP, P2P and HTTP daemons
- Move SIP daemon into its own thread
- Integrate the [Zyre](https://github.com/zeromq/zyre) library for proximity-based Peer to Peer support with tests
- Integrate the [OpenDHT](https://github.com/savoirfairelinux/opendht) library for a 
  [Kademlia](https://en.m.wikipedia.org/wiki/Kademlia) like distributed key-value storage with tests. We will store the 
  public IP addresses of the nodes in the DHT so peers can find each other and share collected data
- Zyre and OpenDHT are optional and detected at build time
- Add `destination_ip` to bad_actor data structure
- Add JSON logging to log bad actor events to a json file via `-j` flag and `SENTRYPEER_JSON_LOG` environment variable, 
  plus custom log file location via `-l` flag and `SENTRYPEER_JSON_LOG_FILE` environment variable
- Add `--disable-opendht/zyre` flags to disable OpenDHT and Zyre build requirements. This allows a user to run 
  SentryPeer as a full standalone node done at build time 

### Tests
- Clean up all valgrind warnings
- Use the new http_daemon_stop() which calls MHD_stop_daemon() to clear up memory leaks, sockets etc. in tests
- Add tests for the sip_daemon_XXXX() functions
- Add tests for the peer_to_peer_lan_XXXX() functions
- Add tests for the peer_to_peer_dht_XXXX() functions
- Switch to cmocka XML output format to show time taken for each test
- Update tests for new JSON log format and new `destination_ip` field in bad_actor data structure

## [1.0.0] - 2022-01-24
- Called Number RESTful API resource live at (`http://x.x.x.x:8082/numbers/{number}`)
- Called Numbers RESTful API resource live at (`http://x.x.x.x:8082/numbers`)

## [0.0.6] - 2022-01-21
- Fix rpm build on Fedora due to wrongly defined libcurl callback definition issue causing `make check` to fail

## [0.0.5] - 2022-01-18
- Switched from 404 to Error 400 on invalid request data at `/ip-addresses/{blah}`
- Handle packets sent that are 0 bytes (discovered by nmap probing port 5060)
- Only parse SIP packets if UDP packet payload is > 0 bytes
- Fedora and Debian packaging now available for each release
- HTTP regex route memory leak fix
- vue.js SPA is now in for start web gui
- Remove duplicate uuid_generate call in bad_actor structure
- Add CORS header to all API responses
- Add more header checks to configure.ac
- Switch to `recvmsg` for receiving UDP packets, so we can save the **destination IP address** of probes
- SIP responsive mode can be enabled to reply to SIP probes. This will prompt **INVITE**'s from bad actors for phone 
  call attempts, so you can collect the phone numbers
- API mode can be set to enable replies to RESTful API requests. Depending on your node deployment type, you may not 
  want to enable this
- Web GUI mode can be set to enable the [Vue.js](https://vuejs.org/) SPA. Depending on your node deployment type, you 
  might not want this
- Add `seen_last` and `seen_count` bad_actor data structure for use in `ip_addresses` RESTful API resource
- `ip_addresses` RESTful API resource results sorted by `seen_last` descending
- Add AC_USE_SYSTEM_EXTENSIONS to configure.ac
- Web GUI mode needs the API to be enabled, so if you don't set API mode, we automatically set it
- Clean up test RESTful API code to fix test suite crashes on macOS
- Highly recommend v5+ of libosip2 via `./configure`
- Correct homebrew installation instructions for SentryPeer
- Major test suite refactor for memory leak fixes
- Replies with json no longer leak memory

## [0.0.4] - 2021-12-22
- Updated README.md copy and paste example for installation instructions from this repository
- Fixed memory leak in route regex matching
- Removed global sqlite error log callback which caused segfaults when trying to show error messages (threads)
- Added an index to the database to speed up searching for IP addresses
- Fixed a segfault when trying to search for IP address/s in an empty database due to global error log bug above
- Better error messages on API responses
- Removed some unnecessary jansson usage
- Dockerfile and now listed on [Docker Hub](https://hub.docker.com/r/sentrypeer/sentrypeer/)

## [0.0.3] - 2021-12-21
- Added `event_uuid`, `collected_method` (passive or responsive) and `created_by_node_id` (aa uuid) columns to `honey` table
- Extended `bad_actor` data structure to provide above
- Added `libuuid` library requirement for `created_by_node_id` and `event_uuid` column of `honey` table
- Memory leak fix with bad_actor not being destroyed
- Added libmicrohttpd, curl and jansson to provide a RESTful API for honeypot data
- Added build requirement for libmicrohttpd and jansson
- Added syslog support for use with [Fail2Ban](https://www.fail2ban.org/wiki/index.php/Main_Page) as per [feature request](https://github.com/SentryPeer/SentryPeer/issues/6)
- Logging to syslog is enabled via `-s` flag. Default is off
- Log `Source IP` to sqlite db and syslog (if enabled) to track probes that aren't SIP compliant
- systemd service file for SentryPeer for Debian/Ubuntu and Fedora
- Debian packaging placeholder branch
- Handle starting up when Web API port is already in use
- Ran `autoscan` and `autoupdate` from autoconf to update configure.ac
- API and Web UI are now available at `http://<host>:8082/`. With a Content-Type of `application/json` SentryPeer responds with JSON as a RESTful API, otherwise it responds with HTML, i.e. the Web UI
- Health check endpoint is now available at `http://<host>:8082/health-check`
- Developer option --with-asan added to configure.ac for ASAN (AddressSanitizer) support during `make check`
- Added environment variable support for all command line options
- The [sqlite](https://www.sqlite.org) `sentrypeer.db` database (call it what you like) location is now configurable via the `-f` flag and `SENTRYPEER_DB_FILE` environment variable. Default is the current working directory `./sentrypeer.db`
- Updated man page
- Add SentryPeer to Sonatype Lift static analysis
- API RESTful resources available are:
  - `/`
  - `/health-check`
  - `/ip-addresses`
  - `/ip-addresses/{ip_address}`
- OpenAPI specification repo is available at https://github.com/SentryPeer/API-Specification with the openapi.json file
  to be served at `http://<host>:8082/openapi.json` (TODO)
- New headers added to API responses:
  - `X-Powered-By`: `SentryPeer`
  - `X-SentryPeer-Version`: `0.0.3`
- `ip_addresses` API endpoint also now shows total number of distinct IP addresses in the database
- `PCRE2` library is now required for building SentryPeer
- All IP address queries now use `inet_pton` to validate IPv4 or IPv6 addresses

## [0.0.2] - 2021-11-24
- Properly exit when `sentrypeer -h` is called
- Correct string errors discovered when building RPMs on Fedora
- RPM specfile
- Man page
- README.md additions

## [0.0.1] - 2021-11-23
- Initial release
- SentryPeer can listen on all interfaces for SIP probes/messages on UDP port 5060 and save them to its sqlite database (*sentrypeer.db*)

