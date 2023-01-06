/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2023 Gavin Henry <ghenry@sentrypeer.org> */
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

#ifndef SENTRYPEER_JSON_LOGGER_H
#define SENTRYPEER_JSON_LOGGER_H 1

#define DEFAULT_JSON_LOG_FILE_NAME "sentrypeer_json.log"

#include "conf.h"
#include "bad_actor.h"

char *bad_actor_to_json(const sentrypeer_config *config,
			const bad_actor *bad_actor_to_convert);
bad_actor *json_to_bad_actor(const sentrypeer_config *config,
			     const char *json_to_convert);
int json_log_bad_actor(const sentrypeer_config *config,
		       const bad_actor *bad_actor);
int json_http_post_bad_actor(const sentrypeer_config *config,
			     const bad_actor *bad_actor);

#endif //SENTRYPEER_JSON_LOGGER_H
