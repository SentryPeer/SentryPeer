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
Version:	0.0.3
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
* Fri Dec 3 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.3-1
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
- Debian packaging placeholder - uncompleted
- Handle starting up when Web API port is already in use
* Wed Nov 25 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.2-1
- First version
