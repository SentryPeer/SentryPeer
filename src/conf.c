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

#define _GNU_SOURCE // for unsetenv
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"
#include "utils.h"
#include "database.h"
#include "json_logger.h"

// Produced by autoconf and cmake (manually by me)
#include "config.h"

//  Constructor
sentrypeer_config *sentrypeer_config_new(void)
{
	sentrypeer_config *self = malloc(sizeof(sentrypeer_config));
	assert(self);

	char *uuid_string = malloc(UTILS_UUID_STRING_LEN);
	assert(uuid_string);

	self->node_id = util_uuid_generate_string(uuid_string);
	self->syslog_mode = false;
	self->json_log_mode = false;
	self->verbose_mode = false;
	self->debug_mode = false;
	self->sip_mode = true; // Default on
	self->sip_responsive_mode = false;
	self->api_mode = false;
	self->web_gui_mode = false;
	self->sip_agent_mode = false;
	self->bgp_agent_mode = false;
	self->p2p_dht_mode = false;
	self->p2p_lan_mode = false;

	self->db_file = calloc(SENTRYPEER_PATH_MAX + 1, sizeof(char));
	assert(self->db_file);
	util_copy_string(self->db_file, DEFAULT_DB_FILE_NAME,
			 SENTRYPEER_PATH_MAX);

	self->json_log_file = calloc(SENTRYPEER_PATH_MAX + 1, sizeof(char));
	assert(self->json_log_file);
	util_copy_string(self->json_log_file, DEFAULT_JSON_LOG_FILE_NAME,
			 SENTRYPEER_PATH_MAX);

	return self;
}

//  Destructor
void sentrypeer_config_destroy(sentrypeer_config **self_ptr)
{
	assert(self_ptr);
	if (*self_ptr) {
		sentrypeer_config *self = *self_ptr;

		// Modern C by Manning, Takeaway 6.19
		// "6.19 Initialization or assignment with 0 makes a pointer null."
		if (self->node_id != 0) {
			free(self->node_id);
			self->node_id = 0;
		}

		if (self->db_file != 0) {
			free(self->db_file);
			self->db_file = 0;
		}

		if (self->json_log_file != 0) {
			free(self->json_log_file);
			self->json_log_file = 0;
		}
		free(self);
		*self_ptr = 0;
	}
}

void print_usage(void)
{
	fprintf(stderr,
		"Usage: %s [-h] [-V] [-w] [-j] [-p] [-f fullpath for sentrypeer.db] [-l fullpath for sentrypeer_json.log] [-r] [-R] [-a] [-s] [-v] [-d]\n",
		PACKAGE_NAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h,      Print this help\n");
	fprintf(stderr, "  -V,      Print version\n");
	fprintf(stderr,
		"  -f,      Set 'sentrypeer.db' location or use SENTRYPEER_DB_FILE env\n");
	fprintf(stderr,
		"  -j,      Enable json logging or use SENTRYPEER_JSON_LOG env\n");
	fprintf(stderr,
		"  -p,      Enable Peer to Peer mode or use SENTRYPEER_PEER_TO_PEER env\n");
	fprintf(stderr,
		"  -a,      Enable RESTful API mode or use SENTRYPEER_API env\n");
	fprintf(stderr,
		"  -w,      Enable Web GUI mode or use SENTRYPEER_WEB_GUI env\n");
	fprintf(stderr,
		"  -r,      Enable SIP responsive mode or use SENTRYPEER_SIP_RESPONSIVE env\n");
	fprintf(stderr,
		"  -R,      Disable SIP mode completely or use SENTRYPEER_SIP_DISABLE env\n");
	fprintf(stderr,
		"  -l,      Set 'sentrypeer_json.log' location or use SENTRYPEER_JSON_LOG_FILE env\n");
	fprintf(stderr,
		"  -s,      Enable syslog logging or use SENTRYPEER_SYSLOG env\n");
	fprintf(stderr,
		"  -v,      Enable verbose logging or use SENTRYPEER_VERBOSE env\n");
	fprintf(stderr,
		"  -d,      Enable debug mode or use SENTRYPEER_DEBUG env\n");
	fprintf(stderr, "\n");
	fprintf(stderr,
		"Report bugs to https://github.com/SentryPeer/SentryPeer/issues\n");
	fprintf(stderr, "\nSee https://sentrypeer.org for more information.\n");
}

void print_version(void)
{
	fprintf(stderr, "This is %s version: %s, git rev: %s\n", PACKAGE_NAME,
		PACKAGE_VERSION, REVISION);
}

int process_cli(sentrypeer_config *config, int argc, char **argv)
{
	int cli_option;

	// Check env vars first
	process_env_vars(config);

	while ((cli_option = getopt(argc, argv, "hVvf:l:jpdrRaws")) != -1) {
		switch (cli_option) {
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
		case 'V':
			print_version();
			exit(EXIT_SUCCESS);
		case 'f':
			if (set_db_file_location(config, optarg) !=
			    EXIT_SUCCESS) {
				fprintf(stderr,
					"Error: Failed to set db file location\n");
				perror("db file location");
				exit(EXIT_FAILURE);
			}
			break;
		case 'l':
			if (set_json_log_file_location(config, optarg) !=
			    EXIT_SUCCESS) {
				fprintf(stderr,
					"Error: Failed to set json log file location\n");
				perror("json log file location");
				exit(EXIT_FAILURE);
			}
			config->json_log_mode = true;
			break;
		case 'a':
			config->api_mode = true;
			break;
		case 'd':
			config->debug_mode = true;
			break;
		case 'r':
			config->sip_responsive_mode = true;
			break;
		case 'R':
			config->sip_mode = false;
			break;
		case 'j':
			config->json_log_mode = true;
			break;
		case 'p':
			config->p2p_dht_mode = true;
			config->p2p_lan_mode = true; // Split into own mode?
			break;
		case 's':
			config->syslog_mode = true;
			break;
		case 'v':
			config->verbose_mode = true;
			break;
		case 'w':
			config->web_gui_mode = true;
			config->api_mode = true;
			break;
		default:
			print_usage();
			return EXIT_FAILURE;
		}
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "SentryPeer node id: %s\n", config->node_id);
	}

	return EXIT_SUCCESS;
}

int process_env_vars(sentrypeer_config *config)
{
	if (getenv("SENTRYPEER_DB_FILE")) {
		util_copy_string(config->db_file, getenv("SENTRYPEER_DB_FILE"),
				 SENTRYPEER_PATH_MAX);
	}
	if (getenv("SENTRYPEER_JSON_LOG_FILE")) {
		util_copy_string(config->json_log_file,
				 getenv("SENTRYPEER_JSON_LOG_FILE"),
				 SENTRYPEER_PATH_MAX);
		config->json_log_mode = true;
	}
	if (getenv("SENTRYPEER_API")) {
		config->api_mode = true;
	}
	if (getenv("SENTRYPEER_WEB_GUI")) {
		config->web_gui_mode = true;
		config->api_mode = true;
	}
	if (getenv("SENTRYPEER_SIP_RESPONSIVE")) {
		config->sip_responsive_mode = true;
	}
	if (getenv("SENTRYPEER_SIP_DISABLE")) {
		config->sip_mode = false;
	}
	if (getenv("SENTRYPEER_SYSLOG")) {
		config->syslog_mode = true;
	}
	if (getenv("SENTRYPEER_JSON_LOG")) {
		config->json_log_mode = true;
	}
	if (getenv("SENTRYPEER_PEER_TO_PEER")) {
		config->p2p_dht_mode = true;
		config->p2p_lan_mode = true;
	}
	if (getenv("SENTRYPEER_VERBOSE")) {
		config->verbose_mode = true;
	}
	if (getenv("SENTRYPEER_DEBUG")) {
		config->debug_mode = true;
	}
	return EXIT_SUCCESS;
}

int set_db_file_location(sentrypeer_config *config, char *cli_db_file_location)
{
	if (cli_db_file_location == NULL) {
		// try to get from the environment
		if (getenv("SENTRYPEER_DB_FILE")) {
			util_copy_string(config->db_file,
					 getenv("SENTRYPEER_DB_FILE"),
					 SENTRYPEER_PATH_MAX);
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"SentryPeer db file location set via SENTRYPEER_DB_FILE env var to: %s\n",
					config->db_file);
			}
		} else {
			// Set to current working directory absolute path with our own db file name
			if (getcwd(config->db_file, SENTRYPEER_PATH_MAX) ==
			    NULL) {
				fprintf(stderr,
					"Error: Failed to get current working directory\n");
				perror("getcwd");
				return EXIT_FAILURE;
			}

			// Build the db file path
			strncat(config->db_file, "/",
				SENTRYPEER_PATH_MAX - strlen(config->db_file));
			strncat(config->db_file, DEFAULT_DB_FILE_NAME,
				SENTRYPEER_PATH_MAX - strlen(config->db_file));
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"SentryPeer db file location set to current working dir: %s\n",
					config->db_file);
			}
		}
		return EXIT_SUCCESS;
	}

	if (cli_db_file_location[0] == '/') {
		util_copy_string(config->db_file, cli_db_file_location,
				 SENTRYPEER_PATH_MAX);
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr,
				"SentryPeer db file location set via cli -f to: %s\n",
				config->db_file);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr,
			"Error: SentryPeer db file location must be an absolute path: %s\n",
			cli_db_file_location);
		return EXIT_FAILURE;
	}
}

// TODO: When we use this for the 3rd time, re-work the lot as it's the same as set_db_file_location
int set_json_log_file_location(sentrypeer_config *config,
			       char *cli_json_log_file_location)
{
	if (cli_json_log_file_location == NULL) {
		// try to get from the environment
		if (getenv("SENTRYPEER_JSON_LOG_FILE")) {
			util_copy_string(config->json_log_file,
					 getenv("SENTRYPEER_JSON_LOG_FILE"),
					 SENTRYPEER_PATH_MAX);
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"SentryPeer json log file location set via SENTRYPEER_JSON_LOG_FILE env var to: %s\n",
					config->json_log_file);
			}
		} else {
			// Set to current working directory absolute path with our own json log file name
			if (getcwd(config->json_log_file,
				   SENTRYPEER_PATH_MAX) == NULL) {
				fprintf(stderr,
					"Error: Failed to get current working directory\n");
				perror("getcwd");
				return EXIT_FAILURE;
			}

			// Build the db file path
			strncat(config->json_log_file, "/",
				SENTRYPEER_PATH_MAX -
					strlen(config->json_log_file));
			strncat(config->json_log_file,
				DEFAULT_JSON_LOG_FILE_NAME,
				SENTRYPEER_PATH_MAX -
					strlen(config->json_log_file));
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"SentryPeer json log file location set to current working dir: %s\n",
					config->json_log_file);
			}
		}
		return EXIT_SUCCESS;
	}

	if (cli_json_log_file_location[0] == '/') {
		util_copy_string(config->json_log_file,
				 cli_json_log_file_location,
				 SENTRYPEER_PATH_MAX);
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr,
				"SentryPeer json log file location set via cli -f to: %s\n",
				config->json_log_file);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr,
			"Error: SentryPeer json log file location must be an absolute path: %s\n",
			cli_json_log_file_location);
		return EXIT_FAILURE;
	}
}
