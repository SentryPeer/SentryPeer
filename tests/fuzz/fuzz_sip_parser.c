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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "src/bad_actor.h"
#include "src/conf.h"
#include "src/sip_parser.h"

extern int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	sentrypeer_config *config = sentrypeer_config_new();
	if (config == NULL) {
		return 0;
	}

	bad_actor *event = bad_actor_new(0, 0, 0, 0, 0, 0, 0, 0, config->node_id);
	if (event != NULL) {
		// Exact-sized, unterminated copy so ASan catches over-reads.
		char *packet = malloc(size ? size : 1);
		if (packet != NULL) {
			if (size > 0) {
				memcpy(packet, data, size);
			}
			sip_message_parser(packet, size, event, config);
			free(packet);
		}
		bad_actor_destroy(&event);
	}

	sentrypeer_config_destroy(&config);
	return 0;
}
