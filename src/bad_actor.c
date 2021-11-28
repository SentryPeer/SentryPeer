/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "bad_actor.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// libosip2 needs these
#include <sys/time.h>
#include <osip2/osip.h>

#include "utils.h"

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
bad_actor *bad_actor_new(char *sip_message, char *source_ip,
			 char *called_number, char *method,
			 char *transport_type, char *user_agent,
			 char *collected_method, char *created_by_node_id)
{
	bad_actor *self = malloc(sizeof(bad_actor));
	assert(self);

	char *time_str = malloc(sizeof(char) * TIMESTAMP_LEN);
	assert(time_str);

	char *uuid_string = malloc(UTILS_UUID_STRING_LEN);
	assert(uuid_string);
	util_uuid_generate_string(uuid_string);

	self->event_timestamp = event_timestamp(time_str);
	self->event_uuid = util_uuid_generate_string(uuid_string);
	self->sip_message = sip_message;
	self->source_ip = source_ip;
	self->called_number = called_number;
	self->method = method;
	self->transport_type = transport_type;
	self->user_agent = user_agent;
	self->collected_method = collected_method;

	if (created_by_node_id) {
		self->created_by_node_id = created_by_node_id;
	} else {
		// TODO: If not set, use the node id of the node that created the event
		// For now, just set it to the event_uuid
		self->created_by_node_id = uuid_string;
	}

	return self;
}

//  Destructor
void bad_actor_destroy(bad_actor **self_ptr)
{
	assert(self_ptr);
	if (*self_ptr) {
		bad_actor *self = *self_ptr;

		if (self->event_timestamp != 0) {
			free(self->event_timestamp);
			self->event_timestamp = 0;
		}

		if (self->event_uuid != 0) {
			free(self->event_uuid);
			self->event_uuid = 0;
		}

		if (self->called_number != 0) {
			free(self->called_number);
			self->called_number = 0;
		}

		if (self->user_agent != 0) {
			free(self->user_agent);
			self->user_agent = 0;
		}

		if (self->method != 0) {
			free(self->method);
			self->method = 0;
		}

		// As per osip_message_to_str();
		if (self->sip_message != 0) {
			osip_free(self->sip_message) self->sip_message = 0;
		}

		free(self);
		*self_ptr = 0;
	}
}
