/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2023 Gavin Henry <ghenry@sentrypeer.org> */
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
#ifndef SENTRYPEER_SIP_PARSER_H
#define SENTRYPEER_SIP_PARSER_H 1

#include <osipparser2/osip_parser.h>
#include "sentrypeer.h"
#include "bad_actor.h"
#include "conf.h"

int sip_message_parser(const char *incoming_sip_msg, size_t packet_size,
		       bad_actor *bad_actor_event,
		       sentrypeer_config const *config);

#endif //SENTRYPEER_SIP_PARSER_H
