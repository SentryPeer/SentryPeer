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
#include "json_logger.h"

#define DHT_PORT 4222
#define DHT_BOOTSTRAP_NODE "bootstrap.sentrypeer.org"
#define DHT_BAD_ACTORS_KEY "bad_actors"

struct op_context {
	dht_runner *runner;
	const sentrypeer_config *config;
};
typedef struct op_context op_context;

static bool dht_value_callback(const dht_value *value, bool expired,
			       void *user_data)
{
	op_context *ctx = user_data;
	const sentrypeer_config *config = ctx->config;
	//dht_runner *runner = ctx->runner;

	dht_data_view data = dht_value_get_data(value);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Value callback %s: %.*s\n",
			expired ? "expired" : "new", (int)data.size, data.data);
		fprintf(stderr, "TODO: Save into into sqlite\n");
	}
	return true;
}

static void dht_done_callback(bool ok, void *user_data)
{
	op_context *ctx = user_data;
	const sentrypeer_config *config = ctx->config;
	//dht_runner *runner = ctx->runner;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Done callback. %s\n",
			ok ? "Success!" : "Failure :-(");
	}
	free(ctx);
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
	dht_infohash_get_from_string(&h, DHT_BAD_ACTORS_KEY);

	if (config->debug_mode) {
		fprintf(stderr, "DHT InfoHash for key '%s' is: %s\n",
			DHT_BAD_ACTORS_KEY, dht_infohash_print(&h));
	}
	config->dht_info_hash = &h; // Save for clean up

	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert(ctx);
	ctx->runner = runner;
	ctx->config = config;

	// Should we dht_runner_get() with dht_get_callback() here to check if
	// the value exists?

	// Listen for data changes on the DHT_PEERS_KEY key
	dht_op_token *token = dht_runner_listen(runner, &h, dht_value_callback,
						op_context_free, ctx);
	assert(token);
	config->dht_op_token = token; // Save for clean up

	// Join the DHT and get our Public IP address
	dht_runner_bootstrap(runner, DHT_BOOTSTRAP_NODE, NULL);

	// Needed? It's in the example code from dhtcnode.c
	sleep(2);

	return EXIT_SUCCESS;
}

int peer_to_peer_dht_save(sentrypeer_config const *config,
			  bad_actor const *bad_actor_event)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Saving bad actor to the DHT...\n");
	}

	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert(ctx);
	ctx->runner = config->dht_node;
	ctx->config = config;

	dht_runner *runner = config->dht_node;
	assert(runner);

	char *bad_actor_json = bad_actor_to_json(config, bad_actor_event);

	dht_value *val = dht_value_new_from_string(bad_actor_json);
	if (val) {
		dht_runner_put(runner, config->dht_info_hash, val,
			       dht_done_callback, ctx, false);
		dht_value_unref(val);
		free(bad_actor_json);
	} else {
		fprintf(stderr, "Failed to create DHT value from string: %s\n",
			bad_actor_json);
		free(bad_actor_json);
		free(ctx);

		return EXIT_FAILURE;
	}

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
