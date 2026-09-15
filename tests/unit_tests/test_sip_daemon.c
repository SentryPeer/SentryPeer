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

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

// Binds a UDP socket to 127.0.0.1:0 (an OS-assigned ephemeral port) so
// sip_send_reply() has a real destination to sendto() and we have a real
// socket to recvfrom() on to see what it actually sent.
static int bind_udp_loopback_socket(struct sockaddr_in *addr_out)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(sock >= 0);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	assert_int_equal(
		bind(sock, (struct sockaddr *)&addr, sizeof(addr)), 0);

	socklen_t addr_len = sizeof(addr);
	assert_int_equal(
		getsockname(sock, (struct sockaddr *)&addr, &addr_len), 0);

	*addr_out = addr;
	return sock;
}

void test_sip_send_reply_correlates_request_headers(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	struct sockaddr_in client_addr;
	int client_sock = bind_udp_loopback_socket(&client_addr);
	int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(server_sock >= 0);

	char const request[] =
		"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-verify-test\r\n"
		"From: <sip:probe@0.0.0.0>;tag=verifytest1\r\n"
		"To: <sip:1000@127.0.0.1>\r\n"
		"Call-ID: verify-test-call-id-12345\r\n"
		"CSeq: 42 OPTIONS\r\n"
		"Content-Length: 0\r\n\r\n";

	sip_message_event *event = sip_message_event_new(
		util_duplicate_string(request), strlen(request), server_sock,
		util_duplicate_string("UDP"), (struct sockaddr *)&client_addr,
		util_duplicate_string("127.0.0.1"), sizeof(client_addr),
		util_duplicate_string("127.0.0.1"));
	assert_non_null(event);

	assert_int_equal(sip_send_reply(config, event), EXIT_SUCCESS);

	char reply[2048] = { 0 };
	ssize_t received = recv(client_sock, reply, sizeof(reply) - 1, 0);
	assert_true(received > 0);

	assert_non_null(strstr(reply, "SIP/2.0 200 OK"));
	assert_non_null(strstr(
		reply,
		"Via: SIP/2.0/UDP 0.0.0.0:55123;branch=z9hG4bK-verify-test"));
	assert_non_null(
		strstr(reply, "From: <sip:probe@0.0.0.0>;tag=verifytest1"));
	assert_non_null(strstr(reply, "Call-ID: verify-test-call-id-12345"));
	assert_non_null(strstr(reply, "CSeq: 42 OPTIONS"));
	// The request's To header had no tag, so the reply must have minted
	// one (RFC 3261 SS8.2.6.2) rather than echo the request verbatim.
	assert_non_null(strstr(reply, "To: <sip:1000@127.0.0.1>;tag="));
	// The stale FreePBX 16/Asterisk 18 fingerprint must be gone.
	assert_null(strstr(reply, "FPBX-16.0.33(18.13.0)"));
	assert_non_null(strstr(reply, "Server: FPBX-17.0.32(22.6.0)"));

	sip_message_event_destroy(&event);
	close(client_sock);
	close(server_sock);
	sentrypeer_config_destroy(&config);
}

void test_sip_send_reply_adds_to_tag_when_missing(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	struct sockaddr_in client_addr;
	int client_sock = bind_udp_loopback_socket(&client_addr);
	int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(server_sock >= 0);

	// This request's To header already carries a tag (as if it were an
	// in-dialog probe) - the reply must preserve it exactly, not mint a
	// second one.
	char const request[] = "OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n"
			       "Via: SIP/2.0/UDP 0.0.0.0:1;branch=z9hG4bK-x\r\n"
			       "From: <sip:probe@0.0.0.0>;tag=abc\r\n"
			       "To: <sip:1000@127.0.0.1>;tag=already-present\r\n"
			       "Call-ID: has-a-to-tag-already\r\n"
			       "CSeq: 1 OPTIONS\r\n"
			       "Content-Length: 0\r\n\r\n";

	sip_message_event *event = sip_message_event_new(
		util_duplicate_string(request), strlen(request), server_sock,
		util_duplicate_string("UDP"), (struct sockaddr *)&client_addr,
		util_duplicate_string("127.0.0.1"), sizeof(client_addr),
		util_duplicate_string("127.0.0.1"));
	assert_non_null(event);

	assert_int_equal(sip_send_reply(config, event), EXIT_SUCCESS);

	char reply[2048] = { 0 };
	ssize_t received = recv(client_sock, reply, sizeof(reply) - 1, 0);
	assert_true(received > 0);

	assert_non_null(
		strstr(reply, "To: <sip:1000@127.0.0.1>;tag=already-present"));
	// Must not have appended a second tag onto the existing one.
	assert_null(strstr(reply, "tag=already-present;tag="));

	sip_message_event_destroy(&event);
	close(client_sock);
	close(server_sock);
	sentrypeer_config_destroy(&config);
}

void test_sip_send_reply_survives_embedded_nul_byte(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	struct sockaddr_in client_addr;
	int client_sock = bind_udp_loopback_socket(&client_addr);
	int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert_true(server_sock >= 0);

	// A malformed packet with an embedded NUL byte inside the SDP body,
	// after otherwise-valid headers - the same packet shape that used to
	// panic the Rust logging path (see the sip.rs NUL fix). This must
	// not crash sip_send_reply() either.
	char request[256];
	size_t header_len = (size_t)snprintf(
		request, sizeof(request),
		"OPTIONS sip:1000@127.0.0.1 SIP/2.0\r\n"
		"Via: SIP/2.0/UDP 0.0.0.0:1;branch=z9hG4bK-nul\r\n"
		"From: <sip:probe@0.0.0.0>;tag=abc\r\n"
		"To: <sip:1000@127.0.0.1>\r\n"
		"Call-ID: has-embedded-nul\r\n"
		"CSeq: 1 OPTIONS\r\n"
		"Content-Length: 4\r\n\r\n");
	request[header_len] = 'A';
	request[header_len + 1] = '\0'; // embedded NUL byte in the body
	request[header_len + 2] = 'B';
	request[header_len + 3] = 'C';
	size_t total_len = header_len + 4;

	char *packet = malloc(total_len);
	assert_non_null(packet);
	memcpy(packet, request, total_len);

	sip_message_event *event = sip_message_event_new(
		packet, total_len, server_sock, util_duplicate_string("UDP"),
		(struct sockaddr *)&client_addr,
		util_duplicate_string("127.0.0.1"), sizeof(client_addr),
		util_duplicate_string("127.0.0.1"));
	assert_non_null(event);

	assert_int_equal(sip_send_reply(config, event), EXIT_SUCCESS);

	char reply[2048] = { 0 };
	ssize_t received = recv(client_sock, reply, sizeof(reply) - 1, 0);
	assert_true(received > 0);
	assert_non_null(strstr(reply, "Call-ID: has-embedded-nul"));

	sip_message_event_destroy(&event);
	close(client_sock);
	close(server_sock);
	sentrypeer_config_destroy(&config);
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
