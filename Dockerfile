# syntax=docker/dockerfile:1
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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
FROM alpine:edge AS builder
#
LABEL maintainer="Gavin Henry, ghenry@sentrypeer.org"
#
RUN apk add --no-cache autoconf automake autoconf-archive \
	git sqlite-dev cmocka-dev util-linux-dev curl-dev \
	pcre2-dev jansson-dev libmicrohttpd-dev build-base \
	libtool rust rust-bindgen clang19-libclang cargo cmake
#
RUN apk add --no-cache -X https://dl-cdn.alpinelinux.org/alpine/edge/testing \
    libosip2-dev
#
RUN apk add --no-cache -X https://dl-cdn.alpinelinux.org/alpine/edge/testing \
    opendht-dev
#
RUN git clone https://github.com/SentryPeer/SentryPeer.git
#
WORKDIR /SentryPeer
#
RUN sed -i '/AM_LDFLAGS=/d' Makefile.am
RUN ./bootstrap.sh
RUN ./configure 
RUN make
RUN make check
RUN make install
# https://github.com/telekom-security/tpotce/blob/22.x/docker/sentrypeer/Dockerfile
FROM alpine:edge
#
LABEL maintainer="Gavin Henry, ghenry@sentrypeer.org"
#
COPY --from=builder /SentryPeer/sentrypeer /opt/sentrypeer/
#
# Install packages (for some reason I'm still looking into, we now need alpine-sdk or 
# a lot of bins, not just sentrypeer, segfault. ldd looks good though 
RUN apk -U add --no-cache \
    libcurl \
    jansson \
    libmicrohttpd \
    libuuid \
    alpine-sdk \
    pcre2 \
    sqlite-libs && \
    apk -U add --no-cache -X https://dl-cdn.alpinelinux.org/alpine/edge/testing \
    libosip2 && \
    apk -U add --no-cache -X https://dl-cdn.alpinelinux.org/alpine/edge/testing \
    opendht-libs && \
    \
    # Setup user, groups and configs
    mkdir -p /var/lib/sentrypeer && \
    mkdir -p /var/log/sentrypeer && \
    addgroup -g 2000 sentrypeer && \
    adduser -S -s /bin/ash -u 2000 -D -g 2000 sentrypeer && \
    chown -R sentrypeer:sentrypeer /opt/sentrypeer /var/lib/sentrypeer /var/log/sentrypeer && \
#
# Clean up
    rm -rf /root/* && \
    rm -rf /var/cache/apk/*
#
# Set workdir and start sentrypeer
STOPSIGNAL SIGKILL
USER sentrypeer:sentrypeer
WORKDIR /opt/sentrypeer/
#
# SIP Port 5060 and SIP TLS 5061
EXPOSE 5060/udp
EXPOSE 5060/tcp
EXPOSE 5061/tcp
EXPOSE 8082/tcp
 #
 # ENV SENTRYPEER_DB_FILE=/my/location/sentrypeer.db
 # ENV SENTRYPEER_API=1
 # ENV SENTRYPEER_WEBHOOK=1
 # ENV SENTRYPEER_WEBHOOK_URL=https://my.webhook.url/events
 # ENV SENTRYPEER_OAUTH2_CLIENT_ID=1234567890
 # ENV SENTRYPEER_OAUTH2_CLIENT_SECRET=1234567890
 # ENV SENTRYPEER_SIP_RESPONSIVE=1
 # ENV SENTRYPEER_SIP_DISABLE=1
 # ENV SENTRYPEER_SYSLOG=1
 # ENV SENTRYPEER_PEER_TO_PEER=1
 # ENV SENTRYPEER_BOOTSTRAP_NODE=mybootstrapnode.com
 # ENV SENTRYPEER_JSON_LOG=1
 # ENV SENTRYPEER_JSON_LOG_FILE=/my/location/sentrypeer_json.log
 # ENV SENTRYPEER_VERBOSE=1
 # ENV SENTRYPEER_DEBUG=1
 # ENV SENTRYPEER_CERT=/my/location/sentrypeer-crt.pem
 # ENV SENTRYPEER_KEY=/my/location/sentrypeer-key.pem
 # ENV SENTRYPEER_TLS_LISTEN_ADDRESS=0.0.0.0:5061
 #
CMD ["./sentrypeer", "-rav", "-f", "/var/lib/sentrypeer/sentrypeer.db"]

