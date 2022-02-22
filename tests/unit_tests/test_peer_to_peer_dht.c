/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
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
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <opendht/opendht_c.h>
#include <config.h>

#include "test_peer_to_peer_dht.h"

#define DHT_PORT 4222
#define DHT_BOOTSTRAP_NODE "bootstrap.jami.net"

struct op_context {
	dht_runner *runner;
	int d;
};

static bool dht_value_callback(const dht_value *value, bool expired,
			       void *user_data)
{
	dht_data_view data = dht_value_get_data(value);
	assert_non_null(data.data);

	fprintf(stderr, "Value callback %s: %.*s.\n",
		expired ? "expired" : "new", (int)data.size, data.data);

	return true;
}

static bool dht_get_callback(const dht_value *value, void *user_data)
{
	dht_runner *runner = (dht_runner *)user_data;
	assert_non_null(runner);

	dht_data_view data = dht_value_get_data(value);
	fprintf(stderr, "Get callback: %.*s.\n", (int)data.size, data.data);
	return true;
}

static void dht_done_callback(bool ok, void *user_data)
{
	dht_runner *runner = (dht_runner *)user_data;
	assert_non_null(runner);

	fprintf(stderr, "Done callback. %s\n",
		ok ? "Success !" : "Failure :-(");
}

static void op_context_free(void *user_data)
{
	struct op_context *ctx = (struct op_context *)user_data;
	fprintf(stderr, "op_context_free %d.\n", ctx->d);
	free(ctx);
}

static char *print_addr(const struct sockaddr *addr)
{
	char *s = NULL;
	switch (addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
		s = malloc(INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
		break;
	}
	case AF_INET6: {
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
		s = malloc(INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s,
			  INET6_ADDRSTRLEN);
		break;
	}
	default:
		break;
	}
	return s;
}

void test_peer_to_peer_dht(void **state)
{
	(void)state; /* unused */

	dht_identity id = dht_identity_generate(PACKAGE_NAME, NULL);
	dht_infohash cert_id = dht_certificate_get_id(id.certificate);
	fprintf(stderr, "Cert ID: %s\n", dht_infohash_print(&cert_id));

	dht_publickey *pk = dht_certificate_get_publickey(id.certificate);
	assert_non_null(pk);

	dht_infohash pk_id = dht_publickey_get_id(pk);

	fprintf(stderr, "PK ID: %s\n", dht_infohash_print(&pk_id));
	dht_publickey_delete(pk);

	pk = dht_privatekey_get_publickey(id.privatekey);
	pk_id = dht_publickey_get_id(pk);
	fprintf(stderr, "Key ID: %s\n", dht_infohash_print(&pk_id));
	dht_publickey_delete(pk);

	dht_identity_delete(&id);

	dht_runner *runner = dht_runner_new();
	assert_non_null(runner);

	dht_runner_run(runner, DHT_PORT);

	dht_infohash h;
	dht_infohash_random(&h);

	fprintf(stderr, "random hash: %s\n", dht_infohash_print(&h));

	// Put data
	const char data_str[] = "yo, this is some data";
	dht_value *val = dht_value_new((const uint8_t*)data_str, strlen(data_str));
	assert_non_null(val);

	dht_runner_put(runner, &h, val, dht_done_callback, runner, false);
	dht_value_unref(val);

	// Get data
	dht_runner_get(runner, &h, dht_get_callback, dht_done_callback, runner);

	// Listen for data
	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert_non_null(ctx);

	ctx->runner = runner;
	ctx->d = 42;

	dht_op_token *token = dht_runner_listen(runner, &h, dht_value_callback,
						op_context_free, ctx);
	assert_non_null(token);

	sleep(1);

	dht_runner_bootstrap(runner, DHT_BOOTSTRAP_NODE, NULL);

	sleep(2);

	struct sockaddr **addrs = dht_runner_get_public_address(runner);
	assert_non_null(addrs);

	for (struct sockaddr **addrIt = addrs; *addrIt; addrIt++) {
		struct sockaddr *addr = *addrIt;
		char *addr_str = print_addr(addr);
		free(addr);
		fprintf(stderr, "Found public address: %s\n", addr_str);
		free(addr_str);
	}
	free(addrs);

	dht_runner_cancel_listen(runner, &h, token);
	dht_op_token_delete(token);

	dht_runner_delete(runner);
}
