/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_SIP_PARSER_H
#define SENTRYPEER_SIP_PARSER_H 1

#include <osipparser2/osip_parser.h>
#include "sentrypeer.h"

int sip_message_parser(char *incoming_sip_msg, size_t packet_size, struct bad_actor *bad_actor);

#endif //SENTRYPEER_SIP_PARSER_H
