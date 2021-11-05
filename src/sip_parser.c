/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "sip_parser.h"
#include "sentrypeer.h"

// http://www.antisip.com/doc/osip2/group__howto__parser.html
int sip_message_parser(char *incoming_sip_message, size_t packet_size)
{
	osip_message_t *parsed_sip_message = NULL;

	if ((osip_message_init(&parsed_sip_message)) < 0) {
		fprintf(stderr, "Cannot initialise osip message lib.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}

	if ((osip_message_parse(parsed_sip_message, incoming_sip_message,
				packet_size)) < 0) {
		fprintf(stderr, "Cannot parse incoming SIP message.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
