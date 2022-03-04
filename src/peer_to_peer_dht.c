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

#define DHT_PORT 4222
#define DHT_BOOTSTRAP_NODE "bootstrap.sentrypeer.org"

int peer_to_peer_dht_run(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting peer to peer DHT mode...\n");
	}

	dht_runner *runner = dht_runner_new();
	assert(runner);

	dht_runner_run(runner, DHT_PORT);

	config->dht_node = runner;

	return EXIT_SUCCESS;
}

int peer_to_peer_dht_stop(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping peer to peer DHT daemon...\n");
	}

	dht_runner_delete(config->dht_node);

	return EXIT_SUCCESS;
}

#else
typedef int make_iso_compilers_happy;
#endif // HAVE_OPENDHT_C
