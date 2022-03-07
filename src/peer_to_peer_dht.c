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

#include <opendht/opendht_c.h>
#include "conf.h"
#include "utils.h"

#define DHT_PORT 4222
#define DHT_BOOTSTRAP_NODE "bootstrap.sentrypeer.org"
#define DHT_PEERS_KEY "peers"

struct op_context {
	dht_runner *runner;
	sentrypeer_config *config;
};
typedef struct op_context op_context;

static bool dht_value_callback(const dht_value *value, bool expired,
			       void *user_data)
{
	op_context *ctx = user_data;
	sentrypeer_config *config = ctx->config;
	//dht_runner *runner = ctx->runner;

	dht_data_view data = dht_value_get_data(value);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Value callback %s: %.*s\n",
			expired ? "expired" : "new", (int)data.size, data.data);
	}
	return true;
}

static bool dht_get_callback(const dht_value *value, void *user_data)
{
	op_context *ctx = user_data;
	sentrypeer_config *config = ctx->config;
	//dht_runner *runner = ctx->runner;

	dht_data_view data = dht_value_get_data(value);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Get callback: %.*s\n", (int)data.size,
			data.data);
	}
	return true;
}

static void dht_done_callback(bool ok, void *user_data)
{
	op_context *ctx = user_data;
	sentrypeer_config *config = ctx->config;
	//dht_runner *runner = ctx->runner;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Done callback. %s\n",
			ok ? "Success!" : "Failure :-(");
	}
}

static void op_context_free(void *user_data)
{
	struct op_context *ctx = (struct op_context *)user_data;
	free(ctx);
}

int peer_to_peer_dht_run(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting peer to peer DHT mode...\n");
	}

	dht_runner *runner = dht_runner_new();
	assert(runner);

	dht_runner_run(runner, DHT_PORT);

	config->dht_node = runner;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Peer to peer DHT mode started.\n");
	}

	// Generate our InfoHash from our key name
	dht_infohash h;
	dht_infohash_get_from_string(&h, DHT_PEERS_KEY);

	if (config->debug_mode) {
		fprintf(stderr, "DHT InfoHash for key '%s' is: %s\n",
			DHT_PEERS_KEY, dht_infohash_print(&h));
	}
	config->dht_info_hash = &h; // Save for clean up

	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert(ctx);
	ctx->runner = runner;
	ctx->config = config;

	// TODO: UDP Hole punching
	// Put our IP address and Port in the DHT for other peers to find us under
	// key DHT_PEERS_KEY
	const char *my_ip_address =
		"my_ip_address"; // TODO: Get this after dht_runner_bootstrap()
	dht_value *val = dht_value_new_from_string(my_ip_address);
	if (val) {
		dht_runner_put(runner, &h, val, dht_done_callback, ctx, false);
		dht_value_unref(val);

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Saving our IP on the DHT: %s\n",
				my_ip_address);
		}
	}

	// Get data to verify it is on the DHT
	dht_runner_get(runner, &h, dht_get_callback, dht_done_callback, ctx);

	// TODO: When this key expires, we need to lookup our IP address again and re-save on the DHT
	// Listen for data changes on the DHT_PEERS_KEY key
	dht_op_token *token = dht_runner_listen(runner, &h, dht_value_callback,
						op_context_free, ctx);
	assert(token);
	config->dht_op_token = token; // Save for clean up

	// Join the DHT and get our Public IP
	dht_runner_bootstrap(runner, DHT_BOOTSTRAP_NODE, NULL);

	sleep(2);

	struct sockaddr **addrs = dht_runner_get_public_address(runner);
	for (struct sockaddr **addrIt = addrs; *addrIt; addrIt++) {
		struct sockaddr *addr = *addrIt;
		char *addr_str = util_addr_string(addr);
		free(addr);
		printf("Found public address: %s\n", addr_str);
		free(addr_str);
	}
	free(addrs);

	return EXIT_SUCCESS;
}

int peer_to_peer_dht_stop(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping peer to peer DHT daemon...\n");
	}

	dht_runner_cancel_listen(config->dht_node, config->dht_info_hash,
				 config->dht_op_token);
	dht_op_token_delete(config->dht_op_token);
	dht_runner_delete(config->dht_node);

	return EXIT_SUCCESS;
}

#else
typedef int make_iso_compilers_happy;
#endif // HAVE_OPENDHT_C
