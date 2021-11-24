# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
# Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org>

Name:		sentrypeer
Version:	0.0.2
Release:	1%{?dist}
Summary:	This is a SIP honeypot for VoIP

License:	GPLv2 or GPLv3
URL:		https://sentrypeer.org
Source0:	https://github.com/SentryPeer/SentryPeer/releases/download/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:	gcc
BuildRequires:	make
BuildRequires:	libcmocka-devel
BuildRequires:	libosip2-devel
BuildRequires:	sqlite-devel

%description	
SentryPeer is a distributed peer to peer list of bad IP addresses and
phone numbers collected via a SIP Honeypot.

%prep
%autosetup

%build
%configure
%make_build

%install
rm -rf $RPM_BUILD_ROOT
%make_install

%check
make check

%files
%license LICENSE.GPL-2.0-only LICENSE.GPL-3.0-only COPYING
%doc AUTHORS CHANGELOG.md README.md COPYRIGHT
%{_bindir}/%{name}
%{_mandir}/man1/%{name}.1.gz

%changelog
* Wed Nov 24 2021 Gavin Henry <ghenry@sentrypeer.org> 0.0.2-1
- First version
