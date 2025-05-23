/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/

// See https://github.com/codeplea/Hands-On-Network-Programming-with-C/blob/master/chap04/udp_recvfrom.c
// and https://www.packtpub.com/product/hands-on-network-programming-with-c/9781789349863 Page 118

#ifndef SENTRYPEER_SIP_DAEMON_H
#define SENTRYPEER_SIP_DAEMON_H 1

#include "conf.h"
#include "bad_actor.h"
#include "sip_message_event.h"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)

#define GETSOCKETERRNO() (errno)
#define SIP_DAEMON_PORT "5060"

int sip_log_event(sentrypeer_config *config,
		  sip_message_event const *sip_event);
int sip_send_reply(sentrypeer_config const *config,
		   sip_message_event const *sip_event);
int sip_daemon_init(sentrypeer_config *config);
int sip_daemon_run(sentrypeer_config *config);
int sip_daemon_stop(sentrypeer_config const *config);

#endif //SENTRYPEER_SIP_DAEMON_H
