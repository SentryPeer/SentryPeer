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

#define _GNU_SOURCE // for setenv
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_conf.h"
#include "../../src/conf.h"

#include <stdlib.h>
#include <uuid/uuid.h>
#include <stdio.h>

void test_conf(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

	// Check uuid_parse always works as we rely on this when validating
	// bad_actors received on the DHT.
	uuid_t node_id;
	char too_short_uuid[] = "baa3db34-1670-441b-baff-32445081509";
	assert_int_equal(uuid_parse(config->node_id, node_id), EXIT_SUCCESS);
	assert_int_equal(uuid_parse(too_short_uuid, node_id), -1);

	// Test that we reject this db file location
	char cli_db_file_location_wrong[] =
		"tests/unit_tests/test_sentrypeer.db";
	assert_int_equal(set_db_file_location(config,
					      cli_db_file_location_wrong),
			 EXIT_FAILURE);

	// Test that we accept a valid db file location
	char cli_db_file_location[] = "/tests/unit_tests/test_sentrypeer.db";
	assert_int_equal(set_db_file_location(config, cli_db_file_location),
			 EXIT_SUCCESS);

	assert_int_equal(setenv("SENTRYPEER_DB_FILE", cli_db_file_location, 1),
			 EXIT_SUCCESS);
	// Test that we set our db file location via the environment variable SENTRYPEER_DB_FILE
	assert_int_equal(set_db_file_location(config, NULL), EXIT_SUCCESS);
	assert_string_equal(config->db_file, cli_db_file_location);

	assert_int_equal(unsetenv("SENTRYPEER_DB_FILE"), EXIT_SUCCESS);
	// Test that we set our own db file location to cwd of runner
	assert_int_equal(set_db_file_location(config, NULL), EXIT_SUCCESS);

	// TODO: Move these into their own test file or JSON group?
	// Test that we reject this json log file location
	char cli_json_log_file_location_wrong[] =
		"tests/unit_tests/test_sentrypeer_json.log";
	assert_int_equal(set_json_log_file_location(
				 config, cli_json_log_file_location_wrong),
			 EXIT_FAILURE);

	// Test that we accept a valid json log file location
	char cli_json_log_file_location[] =
		"/tests/unit_tests/test_sentrypeer_json.log";
	assert_int_equal(set_json_log_file_location(config,
						    cli_json_log_file_location),
			 EXIT_SUCCESS);

	assert_int_equal(setenv("SENTRYPEER_JSON_LOG_FILE",
				cli_json_log_file_location, 1),
			 EXIT_SUCCESS);
	// Test that we set our json log file location via the environment variable SENTRYPEER_JSON_LOG_FILE
	assert_int_equal(set_json_log_file_location(config, NULL),
			 EXIT_SUCCESS);
	assert_string_equal(config->json_log_file, cli_json_log_file_location);

	assert_int_equal(unsetenv("SENTRYPEER_JSON_LOG_FILE"), EXIT_SUCCESS);
	// Test that we set our own json log file location to cwd of runner
	assert_int_equal(set_json_log_file_location(config, NULL),
			 EXIT_SUCCESS);

	// Test non db file env vars
	assert_int_equal(setenv("SENTRYPEER_DEBUG", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->debug_mode);

	assert_int_equal(setenv("SENTRYPEER_VERBOSE", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->verbose_mode);

	assert_int_equal(setenv("SENTRYPEER_SIP_RESPONSIVE", "1", 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->sip_responsive_mode);

	assert_int_equal(setenv("SENTRYPEER_SIP_DISABLE", "1", 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_false(config->sip_mode);

	assert_int_equal(setenv("SENTRYPEER_API", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->api_mode);

	assert_int_equal(setenv("SENTRYPEER_SYSLOG", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->syslog_mode);

	assert_int_equal(setenv("SENTRYPEER_JSON_LOG", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->json_log_mode);

	assert_int_equal(setenv("SENTRYPEER_PEER_TO_PEER", "1", 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->p2p_dht_mode);

	// Check default bootstrap node is set
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->p2p_bootstrap_node,
			    SENTRYPEER_BOOTSTRAP_NODE);

	// Check that we revert to default bootstrap node if DNS name is too long
	char boostrap_node_too_big[300];
	sprintf(boostrap_node_too_big, "%*s", 299, SENTRYPEER_BOOTSTRAP_NODE);

	assert_int_equal(setenv("SENTRYPEER_BOOTSTRAP_NODE",
				boostrap_node_too_big, 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->p2p_bootstrap_node,
			    SENTRYPEER_BOOTSTRAP_NODE);

	// Set our own node
	char bootstrap_node[] = "bootstrap.example.com";
	assert_int_equal(setenv("SENTRYPEER_BOOTSTRAP_NODE", bootstrap_node, 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->p2p_bootstrap_node, bootstrap_node);

	// Check default webhook URL is set (dev URL), but we only actually use it
	// if a user set via -w as config->webhook_mode gets enabled then.
	// Nothing sneaky here.
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->webhook_url, SENTRYPEER_WEBHOOK_URL);

	// Set our own WebHook URL
	char webhook_url[] = "https://webhook.example.com/events";
	assert_int_equal(setenv("SENTRYPEER_WEBHOOK_URL", webhook_url, 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->webhook_url, webhook_url);
	assert_true(config->webhook_mode);

	// Check default OAuth2 client ID and secret are set
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->oauth2_client_id,
			    SENTRYPEER_OAUTH2_CLIENT_ID);
	assert_string_equal(config->oauth2_client_secret,
			    SENTRYPEER_OAUTH2_CLIENT_SECRET);

	// Set OAuth2 client ID
	char oauth2_client_id[] = "my-client-id";
	assert_int_equal(setenv("SENTRYPEER_OAUTH2_CLIENT_ID", oauth2_client_id,
				1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->oauth2_client_id, oauth2_client_id);
	assert_true(config->oauth2_mode);

	// Set OAuth2 client secret
	char oauth2_client_secret[] = "my-client-secret";
	assert_int_equal(setenv("SENTRYPEER_OAUTH2_CLIENT_SECRET",
				oauth2_client_secret, 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_string_equal(config->oauth2_client_secret, oauth2_client_secret);
	assert_true(config->oauth2_mode);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}
