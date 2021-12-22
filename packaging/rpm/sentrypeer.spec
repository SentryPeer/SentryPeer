# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
#
#   _____            _              _____
#  / ____|          | |            |  __ \
# | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
#  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
#  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
# |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
#                              __/ |
#                             |___/
#

Name:		sentrypeer
Version:	0.0.4
Release:	1%{?dist}
Summary:	SIP peer to peer honeypot for VoIP

License:	GPLv2 or GPLv3
URL:		https://sentrypeer.org
Source0:	https://github.com/SentryPeer/SentryPeer/releases/download/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:	gcc
BuildRequires:	make
BuildRequires:	libcmocka-devel
BuildRequires:	libosip2-devel
BuildRequires:	sqlite-devel
BuildRequires:	libuuid-devel
BuildRequires:	libmicrohttpd-devel

%description	
SentryPeer is a distributed peer to peer list of bad IP addresses and
phone numbers collected via a SIP Honeypot.

%prep
%autosetup

%build
%configure
%make_build

%install
%make_install

%check
make check

%files
%license LICENSE.GPL-2.0-only LICENSE.GPL-3.0-only COPYING
%doc AUTHORS CHANGELOG.md README.md COPYRIGHT
%{_bindir}/%{name}
%{_mandir}/man1/%{name}.1*

%changelog
* Wed Dec 22 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.4-1
- Updated README.md copy and paste example for installation instructions from this repository
- Fixed memory leak in route regex matching
- Removed global sqlite error log callback which caused segfaults when trying to show error messages (threads)
- Added an index to the database to speed up searching for IP addresses
- Fixed a segfault when trying to search for IP address/s in an empty database due to global error log bug above
- Better error messages on API responses
- Removed some unnecessary jansson usage
* Tue Dec 21 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.3-1
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
* Wed Nov 25 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.2-1
- First version
