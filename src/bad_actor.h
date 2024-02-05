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

#ifndef SENTRYPEER_BAD_ACTOR_H
#define SENTRYPEER_BAD_ACTOR_H 1

#include "utils.h"
#include "conf.h"
#include <stdint.h>

// Modern C - Manning. Chapter 6, Section 6.4:
// Takeaway 6.29
// (Forward-declare a struct within a typedef using the same identifier
// as the tag name.
//

#define BAD_ACTOR_NOT_FOUND "NOT_FOUND"

typedef struct bad_actor bad_actor;
struct bad_actor {
	char *event_timestamp;
	char *event_uuid;
	char *collected_method;
	char *created_by_node_id;
	char *sip_message;
	char *source_ip;
	char *destination_ip;
	char *called_number;
	char *method;
	char *transport_type;
	char *user_agent;
	char *seen_last;
	char *seen_count;
};

//  Constructor
bad_actor *bad_actor_new(char *sip_message, char *source_ip,
			 char *destination_ip, char *called_number,
			 char *method, char *transport_type, char *user_agent,
			 char *collected_method, char *created_by_node_id);

// Log our bad actor to various places
int bad_actor_log(sentrypeer_config *config, const bad_actor *bad_actor_event);

//  Destructors
void bad_actor_destroy(bad_actor **self_ptr);
void bad_actors_destroy(bad_actor **self_ptr, const int64_t *row_count);

#endif //SENTRYPEER_BAD_ACTOR_H
