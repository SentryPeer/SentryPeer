/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

#define _GNU_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test_bad_actor.h"
#include "../../src/bad_actor.h"
#include "../../src/sip_parser.h"

void test_bad_actor(void **state)
{
	(void)state; /* unused */

	char test_invalid_sip_message_to_parse[] =
		"OPTIONS sip:100@23.148.145.71 SIP/2.0\n"
		"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\n"
		"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\n"
		"To: \"sipvicious\" <sip:100@1.1.1.1>\n"
		"Call-ID: 711444933874895842969934\n"
		"CSeq: 1 OPTIONS\n"
		"Contact: <sip:100@23.148.145.71:5084>\n"
		"Accept: application/sdp\n"
		"User-agent: friendly-scanner\n"
		"Max-forwards: 70\n"
		"Content-Length: 0";

	char test_valid_sip_message_to_parse[] =
		"OPTIONS sip:100@23.148.145.71 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\r\n"
		"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\r\n"
		"To: \"sipvicious\" <sip:100@1.1.1.1>\r\n"
		"Call-ID: 711444933874895842969934\r\n"
		"CSeq: 1 OPTIONS\n"
		"Contact: <sip:100@23.148.145.71:5084>\r\n"
		"Accept: application/sdp\r\n"
		"User-agent: friendly-scanner\r\n"
		"Max-forwards: 70\r\n"
		"Content-Length: 0\r\n";

	char test_valid_sip_message_to_parse_no_called_number[] =
		"OPTIONS sip:100@23.148.145.71 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\r\n"
		"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\r\n"
		"To: \"sipvicious\" <sip:@1.1.1.1>\r\n"
		"Call-ID: 711444933874895842969934\r\n"
		"CSeq: 1 OPTIONS\n"
		"Contact: <sip:100@23.148.145.71:5084>\r\n"
		"Accept: application/sdp\r\n"
		"User-agent: friendly-scanner\r\n"
		"Max-forwards: 70\r\n"
		"Content-Length: 0\r\n";

	char test_valid_sip_message_to_parse_no_user_agent[] =
		"OPTIONS sip:100@23.148.145.71 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\r\n"
		"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\r\n"
		"To: \"sipvicious\" <sip:100@1.1.1.1>\r\n"
		"Call-ID: 711444933874895842969934\r\n"
		"CSeq: 1 OPTIONS\n"
		"Contact: <sip:100@23.148.145.71:5084>\r\n"
		"Accept: application/sdp\r\n"
		"Max-forwards: 70\r\n"
		"Content-Length: 0\r\n";

	char test_source_ip[] = "104.149.141.214";
	char test_transport_type[] = "UDP";
	char test_collected_method[] = "passive";

	bad_actor *bad_actor_event =
		bad_actor_new(0, test_source_ip, 0, 0, test_transport_type, 0,
			      test_collected_method, 0);
	assert_non_null(bad_actor_event);
	assert_string_equal(bad_actor_event->source_ip, test_source_ip);
	assert_string_equal(bad_actor_event->transport_type,
			    test_transport_type);

	fprintf(stderr,
		"New bad actor event created at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	struct sentrypeer_config config;
	config.debug_mode = true;
	fprintf(stderr, "debug_mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(
		sip_message_parser(test_invalid_sip_message_to_parse,
				   strlen(test_invalid_sip_message_to_parse),
				   bad_actor_event, &config),
		EXIT_FAILURE);
	fprintf(stderr,
		"Invalid SIP message processed at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(
		sip_message_parser(test_valid_sip_message_to_parse,
				   strlen(test_valid_sip_message_to_parse),
				   bad_actor_event, &config),
		EXIT_SUCCESS);
	fprintf(stderr,
		"Valid SIP message processed at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_string_equal(bad_actor_event->called_number, "100");
	assert_string_equal(bad_actor_event->user_agent, "friendly-scanner");
	assert_string_equal(bad_actor_event->method, "OPTIONS");

	assert_int_equal(
		sip_message_parser(
			test_valid_sip_message_to_parse_no_called_number,
			strlen(test_valid_sip_message_to_parse),
			bad_actor_event, &config),
		EXIT_SUCCESS);
	assert_string_equal(bad_actor_event->called_number,
			    BAD_ACTOR_NOT_FOUND);

	assert_int_equal(sip_message_parser(
				 test_valid_sip_message_to_parse_no_user_agent,
				 strlen(test_valid_sip_message_to_parse),
				 bad_actor_event, &config),
			 EXIT_SUCCESS);
	assert_string_equal(bad_actor_event->user_agent, BAD_ACTOR_NOT_FOUND);

	bad_actor_destroy(&bad_actor_event);
	assert_null(bad_actor_event);
}

void test_bad_actors(void **state)
{
	(void)state; /* unused */

	int64_t row_count = 100;
	bad_actor *bad_actors = calloc(row_count, sizeof(bad_actor));
	assert_non_null(bad_actors);

	int row_num = 0;
	while (row_num < row_count) {
		bad_actors[row_num].source_ip = strdup("127.0.0.1");
		row_num++;
	}
	bad_actors_destroy(&bad_actors, row_count);
	assert_null(bad_actors);
}
