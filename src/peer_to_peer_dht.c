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
#include "jansson.h"
#include "json_logger.h"
#include "database.h"

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

		// Turn back into JSON
		json_error_t error;
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

		json_t *node_id = json_object_get(json, "created_by_node_id");
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
		json_t *event_uuid = json_object_get(json, "event_uuid");
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
				node_id_str);
		}

		// Check rest of bad_actor keys
		// TODO: Use bad_actor type instead of hard coding here
		const char *bad_actor_keys[] = {
			"event_timestamp", "collected_method", "sip_message",
			"source_ip",	   "destination_ip",   "called_number",
			"method",	   "transport_type"
		};

		int loop_control = 0;
		while (loop_control <= 7) {
			json_t *json_value = json_object_get(
				json, bad_actor_keys[loop_control]);
			if (!json_value) {
				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"JSON from DHT is not valid.\n");
					fprintf(stderr,
						"No '%s' key in JSON.\n",
						bad_actor_keys[loop_control]);
				}
				json_decref(json);
				free(received_value_str);
				return true;
			}
		}

		if (!db_bad_actor_exists(event_uuid_str, config)) {
			// TODO: Move this to a dht_save_bad_actor function
			// It's not from us, so it's a new bad_actor we want to save
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Saving new bad_actor from node_id: %s\n",
					node_id_str);
			}

			bad_actor *bad_actor_event = bad_actor_new(
				util_duplicate_string(json_string_value(
					json_object_get(json, "sip_message"))),
				util_duplicate_string(json_string_value(
					json_object_get(json, "source_ip"))),
				util_duplicate_string(json_string_value(
					json_object_get(json,
							"destination_ip"))),
				util_duplicate_string(json_string_value(
					json_object_get(json, "called_number"))),
				util_duplicate_string(json_string_value(
					json_object_get(json, "method"))),
				util_duplicate_string(json_string_value(
					json_object_get(json,
							"transport_type"))),
				util_duplicate_string(json_string_value(
					json_object_get(json, "user_agent"))),
				util_duplicate_string(json_string_value(
					json_object_get(json,
							"collected_method"))),
				(char *)node_id_str);

			// TODO: This is the same as sip_daemon.c line 372. Move to a save_bad_actor function
			if (config->syslog_mode) {
				syslog(LOG_NOTICE,
				       "Source IP: %s, Method: %s, Agent: %s\n",
				       bad_actor_event->source_ip,
				       bad_actor_event->method,
				       bad_actor_event->user_agent);
			}

			if (config->json_log_mode &&
			    (json_log_bad_actor(config, bad_actor_event) !=
			     EXIT_SUCCESS)) {
				fprintf(stderr,
					"Saving bad_actor json to %s failed.\n",
					config->json_log_file);
			}

			if (db_insert_bad_actor(bad_actor_event, config) !=
			    EXIT_SUCCESS) {
				fprintf(stderr,
					"Saving bad actor to db failed\n");
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

	assert(config->dht_node);

	char *bad_actor_json = bad_actor_to_json(config, bad_actor_event);

	dht_value *val = dht_value_new_from_string(bad_actor_json);
	if (val) {
		dht_runner_put(config->dht_node, config->dht_info_hash, val,
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
