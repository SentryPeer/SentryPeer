/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
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

#include "../config.h"

#if HAVE_OPENDHT_C != 0
#include <opendht/opendht_c.h>
#endif

#define SENTRYPEER_PATH_MAX 4096
#define DNS_MAX_LENGTH 256
#define SENTRYPEER_BOOTSTRAP_NODE "bootstrap.sentrypeer.org"
#define SENTRYPEER_WEBHOOK_URL "http://localhost:4000/api/events"
#define SENTRYPEER_OAUTH2_TOKEN_URL "https://authz.sentrypeer.com/oauth/token"
#define SENTRYPEER_OAUTH2_GRANT_TYPE "client_credentials"
#define SENTRYPEER_OAUTH2_AUDIENCE "https://sentrypeer.com/api"
#define SENTRYPEER_OAUTH2_CLIENT_ID "YOUR_CLIENT_ID"
#define SENTRYPEER_OAUTH2_CLIENT_SECRET "YOUR_CLIENT_SECRET"
#define DHT_BAD_ACTORS_KEY "bad_actors"

typedef struct sentrypeer_config sentrypeer_config;
struct sentrypeer_config {
	bool api_mode;
	bool bgp_agent_mode;
	bool debug_mode;
	bool json_log_mode;
	bool p2p_dht_mode;
	bool sip_agent_mode;
	bool sip_mode;
	bool sip_responsive_mode;
	bool syslog_mode;
	bool verbose_mode;
	bool webhook_mode;
	char *webhook_url;
	bool oauth2_mode;
	char *oauth2_client_id;
	char *oauth2_client_secret;
	char *oauth2_access_token;
	char *db_file;
	char *json_log_file;
	char *node_id;
	char *p2p_bootstrap_node;
	pthread_t sip_daemon_thread;
	void *sip_tls_channel;
	struct MHD_Daemon *http_daemon;

#if HAVE_OPENDHT_C != 0
	dht_runner *dht_node;
	dht_infohash *dht_info_hash;
	dht_op_token *dht_op_token;
#endif
	
#if HAVE_RUST != 0
	bool tls_mode;
	char *tls_cert_file;
	char *tls_key_file;
	char *tls_listen_address;
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
