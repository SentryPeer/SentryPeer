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

#define _GNU_SOURCE // for setenv
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_conf.h"
#include "../../src/conf.h"

#include <stdlib.h>

void test_conf(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);
	config->debug_mode = true;

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

	assert_int_equal(setenv("SENTRYPEER_API", "1", 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->api_mode);


	assert_int_equal(setenv("SENTRYPEER_WEB_GUI", "1", 1),
			 EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->web_gui_mode);

	assert_int_equal(setenv("SENTRYPEER_SYSLOG", "1", 1), EXIT_SUCCESS);
	assert_int_equal(process_env_vars(config), EXIT_SUCCESS);
	assert_true(config->syslog_mode);

	sentrypeer_config_destroy(&config);
}
