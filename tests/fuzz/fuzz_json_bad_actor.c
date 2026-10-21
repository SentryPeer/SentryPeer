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
#include "src/json_logger.h"

// json_to_bad_actor() handles JSON received from other peers on the DHT.
extern int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	sentrypeer_config *config = sentrypeer_config_new();
	if (config == NULL) {
		return 0;
	}

	char *json = malloc(size + 1);
	if (json == NULL) {
		sentrypeer_config_destroy(&config);
		return 0;
	}
	if (size > 0) {
		memcpy(json, data, size);
	}
	json[size] = '\0';

	bad_actor *event = json_to_bad_actor(config, json);
	if (event != NULL) {
		char *round_trip = bad_actor_to_json(config, event);
		free(round_trip);
		bad_actor_destroy(&event);
	}

	free(json);
	sentrypeer_config_destroy(&config);
	return 0;
}
