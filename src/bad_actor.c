/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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

#include "bad_actor.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// libosip2 needs these
#include <sys/time.h>
#include <osip2/osip.h>
#include <syslog.h>

#include "database.h"
#include "json_logger.h"
#include "utils.h"

#if HAVE_RUST != 0
#include "sentrypeer_rust.h"
#endif // HAVE_RUST

//  A Bad Actor class.
//  Implemented as a proper wee class using the CZMQ style.
// See https://github.com/booksbyus/zguide/blob/master/examples/C/udplib.c

/*  As per Linux System Programming its best not to cast the pointer:
*
*  Page 297 of:
*
*  Linux System Programming, Second Edition
*  by Robert Love
*
*  Copyright © 2013 Robert Love. All rights reserved.
*
*  Published by O’Reilly Media, Inc., 1005 Gravenstein Highway North,
*  Sebastopol, CA 95472.
*
*  "Some C programmers like to typecast the result of any function that
*   returns a pointer to void, malloc() included. I argue against this
*   practice because it will hide an error if the return value of the
*   function ever changes to something other than a void pointer.
*
*   Moreover, such a typecast also hides a bug if a function is not
*   properly declared. While the former is not a risk with malloc(),
*   the latter certainly is."
*
*/

//  Constructor
// TODO: make this smaller
bad_actor *bad_actor_new(char *sip_message, char *source_ip,
			 char *destination_ip, char *called_number,
			 char *method, char *transport_type, char *user_agent,
			 char *collected_method, char *created_by_node_id)
{
	bad_actor *self = malloc(sizeof(bad_actor));
	assert(self);

	char *time_str = malloc(sizeof(char) * TIMESTAMP_LEN);
	assert(time_str);

	char *uuid_string = malloc(UTILS_UUID_STRING_LEN);
	assert(uuid_string);

	self->event_timestamp = event_timestamp(time_str);
	self->event_uuid = util_uuid_generate_string(uuid_string);
	self->sip_message = sip_message;
	self->source_ip = source_ip;
	self->destination_ip = destination_ip;
	self->called_number = called_number;
	self->method = method;
	self->transport_type = transport_type;
	self->user_agent = user_agent;
	self->collected_method = collected_method;
	self->created_by_node_id = util_duplicate_string(created_by_node_id);
	self->seen_last = 0;
	self->seen_count = 0;

	return self;
}

int bad_actor_log(sentrypeer_config *config, const bad_actor *bad_actor_event)
{
	if (config->syslog_mode) {
		syslog(LOG_NOTICE, "Source IP: %s, Method: %s, Agent: %s\n",
		       bad_actor_event->source_ip, bad_actor_event->method,
		       bad_actor_event->user_agent);
	}

#if HAVE_RUST != 0
	if (config->new_mode == true) {
		if (config->json_log_mode &&
		    (json_log_bad_actor_rs(config, bad_actor_event) !=
		     EXIT_SUCCESS)) {
			fprintf(stderr, "Saving bad_actor json to %s failed.\n",
				config->json_log_file);
			return EXIT_FAILURE;
		}
	}
#else
	if (config->json_log_mode &&
	    (json_log_bad_actor(config, bad_actor_event) != EXIT_SUCCESS)) {
		fprintf(stderr, "Saving bad_actor json to %s failed.\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}
#endif

	if (db_insert_bad_actor(bad_actor_event, config) != EXIT_SUCCESS) {
		fprintf(stderr, "Saving bad actor to db failed\n");
		return EXIT_FAILURE;
	}

#if HAVE_RUST != 0
	if (config->new_mode == true) {
		if (config->webhook_mode &&
		    (json_http_post_bad_actor_rs(config, bad_actor_event) !=
		     EXIT_SUCCESS)) {
			fprintf(stderr,
				"POSTing bad_actor json to URL '%s' failed.\n",
				config->webhook_url);
			// Just log failing WebHook POSTs
			return EXIT_SUCCESS;
		}
	}
#else
	if (config->webhook_mode &&
	    (json_http_post_bad_actor(config, bad_actor_event) !=
	     EXIT_SUCCESS)) {
		fprintf(stderr, "POSTing bad_actor json to URL '%s' failed.\n",
			config->webhook_url);
		// Just log failing WebHook POSTs
		return EXIT_SUCCESS;
	}
#endif

	return EXIT_SUCCESS;
}

//  Destructor
void bad_actor_destroy(bad_actor **self_ptr)
{
	assert(self_ptr);
	if (*self_ptr) {
		bad_actor *self = *self_ptr;

		// Modern C by Manning, Takeaway 6.19
		// "6.19 Initialization or assignment with 0 makes a pointer null."
		if (self->event_timestamp != 0) {
			free(self->event_timestamp);
			self->event_timestamp = 0;
		}

		if (self->event_uuid != 0) {
			free(self->event_uuid);
			self->event_uuid = 0;
		}

		if (self->source_ip != 0) {
			free(self->source_ip);
			self->source_ip = 0;
		}

		if (self->destination_ip != 0) {
			free(self->destination_ip);
			self->destination_ip = 0;
		}

		if (self->transport_type != 0) {
			free(self->transport_type);
			self->transport_type = 0;
		}

		if (self->collected_method != 0) {
			free(self->collected_method);
			self->collected_method = 0;
		}

		if (self->called_number != 0) {
			free(self->called_number);
			self->called_number = 0;
		}

		if (self->method != 0) {
			free(self->method);
			self->method = 0;
		}

		if (self->user_agent != 0) {
			free(self->user_agent);
			self->user_agent = 0;
		}

		// As per osip_message_to_str();
		if (self->sip_message != 0) {
			// cppcheck-suppress unknownMacro
			osip_free(self->sip_message) self->sip_message = 0;
		}

		if (self->seen_last != 0) {
			free(self->seen_last);
			self->seen_last = 0;
		}

		if (self->seen_count != 0) {
			free(self->seen_count);
			self->seen_count = 0;
		}

		if (self->created_by_node_id != 0) {
			free(self->created_by_node_id);
			self->created_by_node_id = 0;
		}

		free(self);
		*self_ptr = 0;
	}
}

void bad_actors_destroy(bad_actor **bad_actors, const int64_t *row_count)
{
	assert(bad_actors);
	if (*bad_actors) {
		int64_t row_num = 0;
		while (row_num < *row_count) {
			bad_actor_destroy(&bad_actors[row_num]);
			row_num++;
		}
	}
}
