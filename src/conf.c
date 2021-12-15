/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

// Produced by autoconf and cmake (manually by me)
#include "config.h"

//  Constructor
sentrypeer_config *sentrypeer_config_new(void)
{
	sentrypeer_config *self = malloc(sizeof(sentrypeer_config));
	assert(self);

	self->db_file = calloc(PATH_MAX + 1, sizeof(char));
	assert(self->db_file);

	/* From SonarCloud:

	  Recommended Secure Coding Practices

 	  * C11 provides, in its annex K, the strncpy_s and the wcsncpy_s that
 	    were designed as safer alternatives to strcpy and wcscpy. It’s not
 	    recommended to use them in all circumstances, because they introduce
 	    a runtime overhead and require to write more code for error
 	    handling, but they perform checks that will limit the consequences
 	    of calling the function with bad arguments.
 	  * Even if your compiler does not exactly support annex K, you probably
 	    have access to similar functions
 	  * If you are using strncpy and wsncpy as a safer version of strcpy and
 	    wcscpy, you should instead consider strcpy_s and wcscpy_s, because
 	    these functions have several shortcomings:
    	        * It’s not easy to detect truncation
	        * Too much work is done to fill the buffer with 0, leading to
	          suboptimal performance
                * Unless manually corrected, the dest string might not be
                  null-terminated
	  * If you want to use strcpy and wcscpy functions and detect if the
	    string was truncated, the pattern is the following:
		* Set the last character of the buffer to null
    		* Call the function
    		* Check if the last character of the buffer is still null

	  See
		OWASP Top 10 2021 Category A6 - Vulnerable and Outdated Components
		OWASP Top 10 2017 Category A9 - Using Components with Known Vulnerabilities
		MITRE, CWE-120 - Buffer Copy without Checking Size of Input ('Classic Buffer Overflow')
		CERT, STR07-C. - Use the bounds-checking interfaces for string manipulation

	 */

	util_copy_string(self->db_file, DEFAULT_DB_FILE_NAME, PATH_MAX);
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
		if (self->db_file != 0) {
			free(self->db_file);
			self->db_file = 0;
		}
		free(self);
		*self_ptr = 0;
	}
}

void print_usage(void)
{
	fprintf(stderr,
		"Usage: %s [-h] [-V] [-f fullpath for sentrypeer.db] [-s] [-v] [-d]\n",
		PACKAGE_NAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h,      Print this help\n");
	fprintf(stderr, "  -V,      Print version\n");
	fprintf(stderr,
		"  -f,      Set 'sentrypeer.db' location or use SENTRYPEER_DB_FILE env\n");
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
	config->syslog_mode = false;
	config->verbose_mode = false;
	config->debug_mode = false;

	// Check env vars first
	process_env_vars(config);

	while ((cli_option = getopt(argc, argv, "hVf:svd")) != -1) {
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
		case 's':
			config->syslog_mode = true;
			break;
		case 'v':
			config->verbose_mode = true;
			break;
		case 'd':
			config->debug_mode = true;
			break;
		default:
			print_usage();
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

int process_env_vars(sentrypeer_config *config)
{
	if (getenv("SENTRYPEER_DB_FILE")) {
		util_copy_string(config->db_file, getenv("SENTRYPEER_DB_FILE"),
				 PATH_MAX);
	}
	if (getenv("SENTRYPEER_SYSLOG")) {
		config->syslog_mode = true;
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
					 PATH_MAX);
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"SentryPeer db file location set via SENTRYPEER_DB_FILE env var to: %s\n",
					config->db_file);
			}
		} else {
			// Set to current working directory absolute path with our own db file name
			if (getcwd(config->db_file, PATH_MAX) == NULL) {
				fprintf(stderr,
					"Error: Failed to get current working directory\n");
				perror("getcwd");
				return EXIT_FAILURE;
			}

			// Build the db file path
			strncat(config->db_file, "/",
				PATH_MAX - strlen(config->db_file));
			strncat(config->db_file, DEFAULT_DB_FILE_NAME,
				PATH_MAX - strlen(config->db_file));
			assert(config->db_file[strlen(config->db_file)] == 0);

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
				 PATH_MAX);
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
