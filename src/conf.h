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

#ifndef SENTRYPEER_CONFIG_H
#define SENTRYPEER_CONFIG_H 1

#include <getopt.h>
#include <stdbool.h>
#include <pthread.h>

#include "config.h"

#if HAVE_ZYRE !=0
#include <zyre.h>
#endif

#if HAVE_OPENDHT_C !=0
#include <opendht/opendht_c.h>
#endif

#define SENTRYPEER_PATH_MAX 4096

typedef struct sentrypeer_config sentrypeer_config;
struct sentrypeer_config {
	bool syslog_mode;
	bool json_log_mode;
	bool verbose_mode;
	bool debug_mode;
	bool sip_responsive_mode;
	bool api_mode;
	bool web_gui_mode;
	bool sip_agent_mode;
	bool bgp_agent_mode;
	bool p2p_dht_mode;
	bool p2p_lan_mode;
	char *db_file;
	char *json_log_file;
	struct MHD_Daemon *http_daemon;
	pthread_t sip_daemon_thread;

#if HAVE_ZYRE !=0
	zyre_t *p2p_node;
#endif

#if HAVE_OPENDHT_C !=0
	dht_runner *dht_node;
#endif

};

//  Constructor
sentrypeer_config *sentrypeer_config_new(void);

//  Destructor
void sentrypeer_config_destroy(sentrypeer_config **self_ptr);

int process_cli(sentrypeer_config *config, int argc, char **argv);

int process_env_vars(sentrypeer_config *config);

int set_db_file_location(sentrypeer_config *config, char *cli_db_file_location);
int set_json_log_file_location(sentrypeer_config *config,
			       char *cli_json_log_file_location);

#endif // SENTRYPEER_CONFIG_H
