/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_BAD_ACTOR_H
#define SENTRYPEER_BAD_ACTOR_H 1

#include "utils.h"

// Modern C - Manning. Chapter 6, Section 6.4:
// Takeaway 6.29
// (Forward-declare a struct within a typedef using the same identifier
// as the tag name.
//

#define BAD_ACTOR_NOT_FOUND "NOT_FOUND"

typedef struct bad_actor bad_actor;
struct bad_actor {
	char *event_timestamp;
	char *sip_message;
	char *source_ip;
	char *called_number;
	char *method;
	char *transport_type;
	char *user_agent;
};

//  Constructor
bad_actor *bad_actor_new(char *sip_message, char *source_ip,
			 char *called_number, char *method,
			 char *transport_type, char *user_agent);

//  Destructor
void bad_actor_destroy(bad_actor **self_ptr);

#endif //SENTRYPEER_BAD_ACTOR_H
