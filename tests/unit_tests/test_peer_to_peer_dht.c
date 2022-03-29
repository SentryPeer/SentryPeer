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

#include <config.h>

#if HAVE_OPENDHT_C != 0

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

#include "test_peer_to_peer_dht.h"
#include "../../src/utils.h"

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
	dht_value *val = dht_value_new_from_string(data_str);
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

	sleep(1);

	dht_runner_bootstrap(runner, DHT_BOOTSTRAP_NODE, NULL);

	sleep(2);

	dht_op_token *token = dht_runner_listen(runner, &h, dht_value_callback,
						op_context_free, ctx);
	assert_non_null(token);

	struct sockaddr **addrs = dht_runner_get_public_address(runner);
	assert_non_null(addrs);

	for (struct sockaddr **addrIt = addrs; *addrIt; addrIt++) {
		struct sockaddr *addr = *addrIt;
		char *addr_str = util_addr_string(addr);
		free(addr);
		fprintf(stderr, "Found public address: %s\n", addr_str);
		free(addr_str);
	}
	free(addrs);

	dht_runner_cancel_listen(runner, &h, token);
	dht_runner_shutdown(runner, NULL, NULL);
	dht_op_token_delete(token);
	dht_runner_delete(runner);
}
#else
typedef int make_iso_compilers_happy;
#endif // HAVE_OPENDHT_C
