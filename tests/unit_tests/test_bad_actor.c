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
#include "test_database.h"
#include "../../src/sip_parser.h"
#include "../../src/json_logger.h"

bad_actor *test_bad_actor_event_new(void)
{
	char test_source_ip[] = "104.149.141.214";
	char test_destination_ip[] = "8.8.8.8";
	char test_transport_type[] = "UDP";
	char test_collected_method[] = "passive";

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

	bad_actor *bad_actor_event =
		bad_actor_new(0, util_duplicate_string(test_source_ip),
			      util_duplicate_string(test_destination_ip), 0, 0,
			      util_duplicate_string(test_transport_type), 0,
			      util_duplicate_string(test_collected_method),
			      config->node_id);
	assert_non_null(bad_actor_event);
	assert_string_equal(bad_actor_event->source_ip, test_source_ip);
	assert_string_equal(bad_actor_event->destination_ip,
			    test_destination_ip);
	assert_string_equal(bad_actor_event->transport_type,
			    test_transport_type);
	assert_string_equal(bad_actor_event->collected_method,
			    test_collected_method);

	fprintf(stderr,
		"New bad actor event created at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	sentrypeer_config_destroy(&config);
	assert_null(config);

	return bad_actor_event;
}

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

	char test_valid_sip_message_to_parse_blank_user_agent[] =
		"OPTIONS sip:100@23.148.145.71 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\r\n"
		"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\r\n"
		"To: \"sipvicious\" <sip:100@1.1.1.1>\r\n"
		"Call-ID: 711444933874895842969934\r\n"
		"CSeq: 1 OPTIONS\n"
		"Contact: <sip:100@23.148.145.71:5084>\r\n"
		"Accept: application/sdp\r\n"
		"User-agent: \r\n"
		"Max-forwards: 70\r\n"
		"Content-Length: 0\r\n";

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

	strncpy(config->db_file, TEST_DB_FILE, SENTRYPEER_PATH_MAX);
	assert_non_null(config->db_file);

	fprintf(stderr, "debug_mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	bad_actor *bad_actor_event1 = test_bad_actor_event_new();
	assert_int_equal(
		sip_message_parser(test_invalid_sip_message_to_parse,
				   strlen(test_invalid_sip_message_to_parse),
				   bad_actor_event1, config),
		EXIT_FAILURE);

	fprintf(stderr,
		"Invalid SIP message processed at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
	bad_actor_destroy(&bad_actor_event1);
	assert_null(bad_actor_event1);

	bad_actor *bad_actor_event2 = test_bad_actor_event_new();
	assert_int_equal(
		sip_message_parser(test_valid_sip_message_to_parse,
				   strlen(test_valid_sip_message_to_parse),
				   bad_actor_event2, config),
		EXIT_SUCCESS);
	fprintf(stderr,
		"Valid SIP message processed at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_string_equal(bad_actor_event2->called_number, "100");
	assert_string_equal(bad_actor_event2->user_agent, "friendly-scanner");
	assert_string_equal(bad_actor_event2->method, "OPTIONS");
	bad_actor_destroy(&bad_actor_event2);
	assert_null(bad_actor_event2);

	bad_actor *bad_actor_event3 = test_bad_actor_event_new();
	assert_int_equal(
		sip_message_parser(
			test_valid_sip_message_to_parse_no_called_number,
			strlen(test_valid_sip_message_to_parse_no_called_number),
			bad_actor_event3, config),
		EXIT_SUCCESS);
	assert_string_equal(bad_actor_event3->called_number,
			    BAD_ACTOR_NOT_FOUND);
	bad_actor_destroy(&bad_actor_event3);
	assert_null(bad_actor_event3);

	bad_actor *bad_actor_event4 = test_bad_actor_event_new();
	assert_int_equal(
		sip_message_parser(
			test_valid_sip_message_to_parse_no_user_agent,
			strlen(test_valid_sip_message_to_parse_no_user_agent),
			bad_actor_event4, config),
		EXIT_SUCCESS);
	assert_string_equal(bad_actor_event4->user_agent, BAD_ACTOR_NOT_FOUND);
	bad_actor_destroy(&bad_actor_event4);
	assert_null(bad_actor_event4);

	bad_actor *bad_actor_event4_1 = test_bad_actor_event_new();
	assert_int_equal(
		sip_message_parser(
			test_valid_sip_message_to_parse_blank_user_agent,
			strlen(test_valid_sip_message_to_parse_blank_user_agent),
			bad_actor_event4_1, config),
		EXIT_SUCCESS);
	assert_string_equal(bad_actor_event4_1->user_agent, BAD_ACTOR_NOT_FOUND);
	bad_actor_destroy(&bad_actor_event4_1);
	assert_null(bad_actor_event4_1);

	bad_actor *bad_actor_event5 = test_bad_actor_event_new();
	char *bad_actor_json = bad_actor_to_json(config, bad_actor_event5);
	assert_non_null(bad_actor_json);
	free(bad_actor_json);

	assert_int_equal(bad_actor_log(config, bad_actor_event5), EXIT_SUCCESS);

	bad_actor_destroy(&bad_actor_event5);
	assert_null(bad_actor_event5);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}

void test_bad_actors(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

	int64_t row_count = 100;
	bad_actor **bad_actors = calloc(row_count, sizeof(*bad_actors));
	assert_non_null(bad_actors);

	int row_num = 0;
	while (row_num < row_count) {
		bad_actors[row_num] =
			bad_actor_new(0, util_duplicate_string("127.0.0.1"), 0,
				      0, 0, 0, 0, 0, config->node_id);
		row_num++;
	}

	assert_string_equal(bad_actors[29]->source_ip, "127.0.0.1");
	bad_actors_destroy(bad_actors, &row_count);
	free(bad_actors);
	bad_actors = 0;
	assert_null(bad_actors);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}
