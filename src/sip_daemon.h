/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

// See https://github.com/codeplea/Hands-On-Network-Programming-with-C/blob/master/chap04/udp_recvfrom.c
// and https://www.packtpub.com/product/hands-on-network-programming-with-c/9781789349863 Page 118

#ifndef SENTRYPEER_SIP_DAEMON_H
#define SENTRYPEER_SIP_DAEMON_H 1

#include "conf.h"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int sip_daemon_init(struct sentrypeer_config const *config);

#endif //SENTRYPEER_SIP_DAEMON_H
