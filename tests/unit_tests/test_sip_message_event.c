/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2026 Gavin Henry <ghenry@sentrypeer.org> */
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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test_sip_message_event.h"
#include "../../src/sip_message_event.h"
#include "../../src/conf.h"
#include "../../src/utils.h"

void test_sip_message_event(void **state)
{
	(void)state; /* unused */

	char test_read_packet_buf[] = "anything";
	size_t test_bytes_received = 10;
	SOCKET test_socket_listen_udp = 0;
	char test_transport_type[] = "TCP";
	SOCKET test_client_address = 0;
	char test_udp_client_ip_address_buffer[] = "X.X.X.X";
	size_t test_client_len = 10;
	char test_dest_ip_address_buffer[] = "X.X.X.X";

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

	sip_message_event *sip_event = sip_message_event_new(
		util_duplicate_string(test_read_packet_buf),
		test_bytes_received, test_socket_listen_udp,
		util_duplicate_string(test_transport_type),
		(struct sockaddr *)&test_client_address,
		util_duplicate_string(test_udp_client_ip_address_buffer),
		test_client_len,
		util_duplicate_string(test_dest_ip_address_buffer));

	assert_non_null(sip_event);

	assert_string_equal(sip_event->packet, test_read_packet_buf);
	assert_int_equal(sip_event->packet_len, test_bytes_received);
	assert_string_equal(sip_event->transport_type, test_transport_type);
	assert_string_equal(sip_event->client_ip_addr_str,
			    test_udp_client_ip_address_buffer);
	assert_int_equal(sip_event->client_addr_len, test_client_len);
	assert_string_equal(sip_event->dest_ip_addr_str,
			    test_dest_ip_address_buffer);

	fprintf(stderr,
		"New sip_message_event created at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	sentrypeer_config_destroy(&config);
	assert_null(config);

	sip_message_event_destroy(&sip_event);
	assert_null(sip_event);
}
