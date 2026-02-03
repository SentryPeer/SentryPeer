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

#include "conf.h"

#if HAVE_OPENDHT_C != 0

#include <opendht/opendht_c.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "jansson.h"
#include "json_logger.h"
#include "database.h"

#define DHT_PORT 4222
#define DHT_BOOTSTRAP_WAIT_TIME 5

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

	if (expired) {
		return true;
	}

	if (data.size == 0) {
		return true;
	}

	// TODO: Move to a validator function
	if (data.size > 0) {
		char *received_value_str = malloc(data.size + 1);
		memcpy(received_value_str, data.data, data.size);
		received_value_str[data.size] = '\0';

		// Turn back into JSON string
		json_error_t error;
		// Maybe switch to json_loadb() instead of json_loads() if needed:
		// https://github.com/savoirfairelinux/opendht/issues/596#issuecomment-1079957048
		json_t *json = json_loads(received_value_str, 0, &error);

		if (!json_is_object(json)) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"JSON from DHT is not valid.\n");
				fprintf(stderr, "Failed to parse: %s\n",
					error.text);
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		const json_t *node_id =
			json_object_get(json, "created_by_node_id");
		if (!node_id) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"JSON from DHT is not valid.\n");
				fprintf(stderr, "No node_id in JSON.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		const char *node_id_str = json_string_value(node_id);
		if (!node_id_str) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"JSON from DHT is not valid.\n");
				fprintf(stderr, "Node ID is not a string.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		uuid_t node_id_uuid_check;
		if (uuid_parse(node_id_str, node_id_uuid_check) !=
		    EXIT_SUCCESS) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Node ID uuid in JSON from DHT is not valid.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Node ID from DHT value is: %s\n",
				node_id_str);
		}

		// Check it's not from us
		if (strncmp(node_id_str, config->node_id,
			    strlen(config->node_id)) == 0) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Node ID from DHT value is the same as ours. Not saving bad_actor.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		// TODO: Validate event_uuid and check our database to see if we already have this bad_actor
		// by using the event_uuid
		const json_t *event_uuid = json_object_get(json, "event_uuid");
		if (!event_uuid) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"JSON from DHT is not valid.\n");
				fprintf(stderr, "No event_uuid in JSON.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		const char *event_uuid_str = json_string_value(event_uuid);
		if (!event_uuid_str) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"JSON from DHT is not valid.\n");
				fprintf(stderr,
					"event_uuid is not a string.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		if (!is_valid_uuid(event_uuid_str)) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"event_uuid in JSON from DHT is invalid.\n");
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "event_uuid from DHT value is: %s\n",
				event_uuid_str);

			fprintf(stderr,
				"Checking we haven't seen this event_uuid before in our db: %s\n",
				event_uuid_str);
		}

		if (!db_bad_actor_exists(event_uuid_str, config)) {
			// TODO: Move this to a dht_save_bad_actor function
			// It's not from us, so it's a new bad_actor we want to save
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Saving new bad_actor from node_id: %s\n",
					node_id_str);
			}

			// Ready to convert JSON to a bad_actor
			bad_actor *bad_actor_event =
				json_to_bad_actor(config, received_value_str);
			if (!bad_actor_event) {
				fprintf(stderr,
					"Converting JSON to a bad_actor failed.\n");
				json_decref(json);
				free(received_value_str);
				return true;
			}

			if (bad_actor_log(config, bad_actor_event) !=
			    EXIT_SUCCESS) {
				fprintf(stderr, "Logging bad_actor failed.\n");
			}

			json_decref(json);
			free(received_value_str);
			bad_actor_destroy(&bad_actor_event);
			return true;
		} else {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"bad_actor event_uuid already exists, not saving: %s\n",
					event_uuid_str);
			}
			json_decref(json);
			free(received_value_str);
			return true;
		}
	}
	return true;
}

static void dht_done_callback(bool ok, void *user_data)
{
	op_context *ctx = user_data;
	assert(ctx);

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
		fprintf(stderr,
			"Starting peer to peer DHT mode using OpenDHT-C lib version '%s'...\n",
			OPENDHT_C_VERSION);
	}

	// https://github.com/savoirfairelinux/opendht/issues/590#issuecomment-1063158916
	dht_runner_config dht_config;
	dht_runner_config_default(&dht_config);

	dht_config.peer_discovery = true; // Look for other peers on the network
	dht_config.peer_publish = true; // Publish our own peer info

	dht_runner *runner = dht_runner_new();
	assert(runner);

	dht_runner_run_config(runner, DHT_PORT, &dht_config);
	config->dht_node = runner;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Peer to peer DHT mode started.\n");
		fprintf(stderr, "DHT InfoHash for key '%s' is: %s\n",
			DHT_BAD_ACTORS_KEY,
			dht_infohash_print(config->dht_info_hash));
	}

	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert(ctx);
	ctx->runner = runner;
	ctx->config = config;

	// Join the DHT network *before* we listen:
	// https://github.com/savoirfairelinux/opendht/issues/596#issuecomment-1079957048
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Bootstrapping the DHT\n");
	}
	dht_runner_bootstrap(runner, config->p2p_bootstrap_node, NULL);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr,
			"Waiting %d seconds for bootstrapping to %s...\n",
			DHT_BOOTSTRAP_WAIT_TIME, config->p2p_bootstrap_node);
	}
	sleep(DHT_BOOTSTRAP_WAIT_TIME);

	// Listen for data changes on the DHT_PEERS_KEY key
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr,
			"Listening for changes to the bad_actors DHT key\n");
	}
	dht_op_token *token =
		dht_runner_listen(runner, config->dht_info_hash,
				  dht_value_callback, op_context_free, ctx);
	assert(token);
	config->dht_op_token = token; // Save for clean up

	return EXIT_SUCCESS;
}

int peer_to_peer_dht_save(sentrypeer_config *config,
			  bad_actor const *bad_actor_event)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Saving bad actor on the DHT...\n");
	}

	struct op_context *ctx = malloc(sizeof(struct op_context));
	assert(ctx);
	ctx->runner = config->dht_node;
	ctx->config = config;

	assert(config->dht_node);

	char *bad_actor_json = bad_actor_to_json(config, bad_actor_event);
	// We don't assert here, because we want to continue even if it fails
	if (bad_actor_json == NULL) {
		fprintf(stderr, "Failed to convert bad actor to json.\n");
		free(bad_actor_json);
		free(ctx);
		return EXIT_FAILURE;
	}

	dht_value *val = dht_value_new_from_string(bad_actor_json);
	if (val) {
		// Make these permanent:
		// https://github.com/savoirfairelinux/opendht/issues/596#issuecomment-1079957048
		dht_runner_put(config->dht_node, config->dht_info_hash, val,
			       dht_done_callback, ctx, true);
		dht_value_unref(val);
		free(bad_actor_json);
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr,
				"bad actor permanently saved on the DHT...\n");
		}
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
	dht_runner_shutdown(config->dht_node, NULL, NULL);
	dht_op_token_delete(config->dht_op_token);
	dht_runner_delete(config->dht_node);

	return EXIT_SUCCESS;
}

#else
typedef int make_iso_compilers_happy;
#endif // HAVE_OPENDHT_C
