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
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../../src/sip_daemon.h"
#include "../../src/sip_message_event.h"
#include "../../src/conf.h"
#include "../../src/utils.h"

#if HAVE_RUST != 0
#include "../../src/sentrypeer_rust.h"
#endif

// Sends `request` through sip_send_reply() and reads the reply from a
// loopback socket. Like the daemon, the event gets a strndup'd copy of the
// request (it ends at the first NUL) and the full length as packet_len.
static void send_reply_roundtrip(char const *request, size_t request_len,
				 char *reply, size_t reply_len)
{
	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(client_sock >= 0);
	struct timeval timeout = { .tv_sec = 2 };
	assert_int_equal(setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO,
				    &timeout, sizeof(timeout)),
			 0);
	struct sockaddr_in client_addr = { 0 };
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	socklen_t addr_len = sizeof(client_addr);
	assert_int_equal(bind(client_sock, (struct sockaddr *)&client_addr,
			      sizeof(client_addr)),
			 0);
	assert_int_equal(getsockname(client_sock,
				     (struct sockaddr *)&client_addr,
				     &addr_len),
			 0);

	int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(server_sock >= 0);

	sip_message_event *event = sip_message_event_new(
		util_duplicate_string(request), request_len, server_sock,
		util_duplicate_string("UDP"), (struct sockaddr *)&client_addr,
		util_duplicate_string("127.0.0.1"), sizeof(client_addr),
		util_duplicate_string("127.0.0.1"));
	assert_non_null(event);

	assert_int_equal(sip_send_reply(config, event), EXIT_SUCCESS);
	memset(reply, 0, reply_len);
	assert_true(recv(client_sock, reply, reply_len - 1, 0) > 0);

	sip_message_event_destroy(&event);
	close(client_sock);
	close(server_sock);
	sentrypeer_config_destroy(&config);
}

static void assert_reply_contains(char const *request, char const *expected)
{
	char reply[2048];

	send_reply_roundtrip(request, strlen(request), reply, sizeof(reply));
	assert_non_null(strstr(reply, expected));
}

void test_sip_send_reply_correlates_request_headers(void **state)
{
	(void)state; /* unused */

	char const request[] =
		"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-x\r\n"
		"From: <sip:probe@0.0.0.0>;tag=abc\r\n"
		"To: <sip:1000@127.0.0.1>\r\n"
		"Call-ID: call-id-1\r\n"
		"CSeq: 42 OPTIONS\r\n"
		"Content-Length: 0\r\n\r\n";
	char reply[2048];

	send_reply_roundtrip(request, strlen(request), reply, sizeof(reply));

	assert_non_null(strstr(reply, "SIP/2.0 200 OK"));
	assert_non_null(strstr(
		reply,
		"Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-x"));
	assert_non_null(
		strstr(reply, "From: <sip:probe@0.0.0.0>;tag=abc"));
	assert_non_null(strstr(reply, "Call-ID: call-id-1"));
	assert_non_null(strstr(reply, "CSeq: 42 OPTIONS"));
	assert_non_null(strstr(reply, "To: <sip:1000@127.0.0.1>;tag="));
	assert_non_null(strstr(reply, "Server: FPBX-17.0.32(22.6.0)"));
}

void test_sip_send_reply_preserves_existing_to_tag(void **state)
{
	(void)state; /* unused */

	char const *to_values[] = { "<sip:1000@127.0.0.1>;tag=abc",
				    "<sip:1000@127.0.0.1>; tag=abc" };

	for (size_t i = 0; i < sizeof(to_values) / sizeof(to_values[0]); i++) {
		char request[128];
		char expected[64];

		snprintf(request, sizeof(request),
			 "OPTIONS sip:x SIP/2.0\r\nTo: %s\r\n\r\n",
			 to_values[i]);
		snprintf(expected, sizeof(expected), "To: %s\r\n",
			 to_values[i]);
		assert_reply_contains(request, expected);
	}
}

void test_sip_send_reply_header_edge_cases(void **state)
{
	(void)state; /* unused */

	char reply[2048];

	// Leading CRLF
	assert_reply_contains("\r\nOPTIONS sip:x SIP/2.0\r\nCall-ID: a\r\n\r\n",
			      "Call-ID: a\r\n");
	// Whitespace around the colon
	assert_reply_contains(
		"OPTIONS sip:x SIP/2.0\r\nVia : SIP/2.0/UDP 1.2.3.4:9\r\n\r\n",
		"Via: SIP/2.0/UDP 1.2.3.4:9\r\n");
	assert_reply_contains("OPTIONS sip:x SIP/2.0\r\nCall-ID:\tfoo\r\n\r\n",
			      "Call-ID: foo\r\n");
	// No colon in the request line
	assert_reply_contains("OPTIONS * SIP/2.0\r\nCall-ID: star\r\n\r\n",
			      "Call-ID: star\r\n");

	// A header in the body is not echoed
	char const in_body[] = "OPTIONS sip:x SIP/2.0\r\nVia: v\r\n\r\n"
			       "Call-ID: body\r\n";
	send_reply_roundtrip(in_body, strlen(in_body), reply, sizeof(reply));
	assert_null(strstr(reply, "Call-ID: body"));

	// An empty value falls back to the default
	assert_reply_contains("OPTIONS sip:x SIP/2.0\r\nCall-ID:\r\n\r\n",
			      "Call-ID: 1179563087@127.0.0.1\r\n");

	// A stray LF ends the value
	char const stray_lf[] = "OPTIONS sip:x SIP/2.0\r\n"
				"Call-ID: x\nContent-Length: 999\r\n\r\n";
	send_reply_roundtrip(stray_lf, strlen(stray_lf), reply, sizeof(reply));
	assert_non_null(strstr(reply, "Call-ID: x\r\n"));
	assert_null(strstr(reply, "Content-Length: 999"));
}

void test_sip_send_reply_ignores_bytes_after_nul(void **state)
{
	(void)state; /* unused */

	// The event's copy ends at this NUL but packet_len is 1100. Run under
	// ASan or valgrind to catch an over-read.
	char request[1100] =
		"OPTIONS sip:x SIP/2.0\r\nCall-ID: has-nul\r\n\r\nA";
	size_t nul = strlen(request);
	char reply[2048];

	memset(request + nul + 1, 'B', sizeof(request) - nul - 1);
	send_reply_roundtrip(request, sizeof(request), reply, sizeof(reply));

	assert_non_null(strstr(reply, "Call-ID: has-nul"));
}

void test_sip_daemon(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	if (config->new_mode == true) {
#if HAVE_RUST != 0
		assert_int_equal(run_sip_server(config), EXIT_SUCCESS);
#endif
	} else {
		assert_int_equal(sip_daemon_run(config), EXIT_SUCCESS);
	}
	assert_int_equal(sip_daemon_stop(config), EXIT_SUCCESS);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}
