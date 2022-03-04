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

#if HAVE_ZYRE !=0

#include <zyre.h>
#include "conf.h"

int peer_to_peer_lan_run(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting peer to peer LAN mode...\n");
	}

	zyre_t *node = zyre_new(NULL);
	assert(node);

	if (zyre_start(node) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to start peer to peer LAN node.\n");
		zyre_destroy(&node);
		return EXIT_FAILURE;
	}
	config->p2p_node = node;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Peer to peer LAN mode started.\n");
		fprintf(stderr, "Joining peer to peer LAN room %s\n",
			PACKAGE_NAME);
	}

	if (zyre_join(node, PACKAGE_NAME) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to join peer to peer LAN room.\n");
		zyre_stop(node);
		zyre_destroy(&node);
		return EXIT_FAILURE;
	}

	if (zyre_shouts(node, PACKAGE_NAME, "Hello from %s", zyre_name(node)) !=
	    EXIT_SUCCESS) {
		fprintf(stderr, "Failed to shout to peer to peer LAN room.\n");
		zyre_stop(node);
		zyre_destroy(&node);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Said hello to peer to peer LAN %s\n",
			PACKAGE_NAME);
	}

	return EXIT_SUCCESS;
}

int peer_to_peer_lan_stop(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping peer to peer LAN daemon...\n");
	}

	zyre_stop(config->p2p_node);
	zyre_destroy(&config->p2p_node);

	return EXIT_SUCCESS;
}
#else
typedef int make_iso_compilers_happy;
#endif // DISABLE_ZYRE
