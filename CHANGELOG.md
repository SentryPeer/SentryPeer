# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
- Switched from 404 to Error 400 on invalid request data at `/ip-addresses/{blah}`

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

