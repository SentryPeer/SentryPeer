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
Source1:	https://raw.githubusercontent.com/SentryPeer/SentryPeer/v%{version}/packaging/rpm/%{name}.options
Source2:	https://raw.githubusercontent.com/SentryPeer/SentryPeer/v%{version}/packaging/rpm/%{name}.service

BuildRequires:	gcc
BuildRequires:	gcc-c++
BuildRequires:	make
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	autoconf-archive
BuildRequires:  systemd
BuildRequires:	libcmocka-devel
BuildRequires:	libosip2-devel
BuildRequires:	sqlite-devel
BuildRequires:	libuuid-devel
BuildRequires:	libmicrohttpd-devel
BuildRequires:	libcurl-devel
BuildRequires:	jansson-devel
BuildRequires:	pcre2-devel
Requires(pre): shadow-utils

%description
SentryPeer is a distributed peer to peer list of bad IP addresses and
phone numbers collected via a SIP Honeypot.

%prep
%autosetup

%build
%configure
%make_build

%pre
getent group %{name} >/dev/null || groupadd -r %{name}
getent passwd %{name} >/dev/null || \
    useradd -r -g %{name} -d /home/%{name} -s /sbin/nologin \
    -c "%{name} system account" %{name}
exit 0

%install
%make_install
install -p -D -m 0644 %{SOURCE1} %{buildroot}%{_sysconfdir}/sysconfig/%{name}
install -p -D -m 0644 %{SOURCE2} %{buildroot}%{_unitdir}/%{name}.service
mkdir -p %{buildroot}%{_sharedstatedir}/%{name}

%check
make check

%post
systemctl enable %{name}.service

%files
%config(noreplace) %{_sysconfdir}/sysconfig/%{name}
%doc AUTHORS CHANGELOG.md README.md COPYRIGHT
%license LICENSE.GPL-2.0-only LICENSE.GPL-3.0-only COPYING
%attr(0700,%{name},%{name}) %dir %{_sharedstatedir}/%{name}
%{_bindir}/%{name}
%{_mandir}/man1/%{name}.1*
%{_unitdir}/%{name}.service

%changelog
* Wed Dec 22 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.4-1
- Updated README.md copy and paste example for installation instructions from this repository
- Fixed memory leak in route regex matching
- Removed global sqlite error log callback which caused segfaults when trying to show error messages (threads)
- Added an index to the database to speed up searching for IP addresses
- Fixed a segfault when trying to search for IP address/s in an empty database due to global error log bug above
- Better error messages on API responses
- Removed some unnecessary jansson usage
- Switched from 404 to Error 400 on invalid request data at `/ip-addresses/{blah}`
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
* Thu Nov 25 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.2-1
- First version
