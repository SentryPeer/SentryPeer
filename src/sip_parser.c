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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <assert.h>

#include "sip_parser.h"
#include "conf.h"

// http://www.antisip.com/doc/osip2/group__howto__parser.html

int sip_message_parser(const char *incoming_sip_message, size_t packet_size,
		       bad_actor *bad_actor_event,
		       sentrypeer_config const *config)

{
	osip_message_t *parsed_sip_message = 0;

	if ((osip_message_init(&parsed_sip_message)) < 0) {
		fprintf(stderr, "Cannot initialise osip message lib.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}

	// https://stackoverflow.com/a/36957123/1072411
	//
	// This is needed and not documented in the osip2 library. Argh!!!!
	if ((parser_init()) < 0) {
		fprintf(stderr, "Cannot initialise osip parser.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}

	// Parse the whole SIP message
	if ((osip_message_parse(parsed_sip_message, incoming_sip_message,
				packet_size)) < 0) {
		fprintf(stderr, "Cannot parse incoming SIP message.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}

	// Full SIP Message
	size_t sip_message_length = SIP_MESSAGE_MAX_LENGTH;
	bad_actor_event->sip_message = 0; // Clear the previous SIP message
	if (osip_message_to_str(parsed_sip_message,
				&bad_actor_event->sip_message,
				&sip_message_length) != 0) {
		fprintf(stderr, "Cannot convert SIP message to string.\n");
		osip_message_free(parsed_sip_message);
		return EXIT_FAILURE;
	}

	// SIP Method
	bad_actor_event->method = 0; // Clear the previous SIP method
	bad_actor_event->method =
		util_duplicate_string(parsed_sip_message->sip_method);

	// Phone Number called
	bad_actor_event->called_number = 0; // Clear the previous called number
	if (parsed_sip_message->to != NULL) {
		if (parsed_sip_message->to->url != NULL) {
			if (parsed_sip_message->to->url->username != NULL) {
				bad_actor_event
					->called_number = util_duplicate_string(
					parsed_sip_message->to->url->username);
			} else {
				bad_actor_event->called_number =
					util_duplicate_string(
						BAD_ACTOR_NOT_FOUND);
			}
		} else {
			bad_actor_event->called_number =
				util_duplicate_string(BAD_ACTOR_NOT_FOUND);
		}
	} else {
		bad_actor_event->called_number =
			util_duplicate_string(BAD_ACTOR_NOT_FOUND);
	}

	// SIP User Agent
	bad_actor_event->user_agent = 0; // Clear the previous SIP user agent
	osip_header_t *user_agent_header = 0;
	osip_message_get_user_agent(parsed_sip_message, 0, &user_agent_header);
	if (user_agent_header != NULL && user_agent_header->hvalue != NULL &&
	    strcmp(user_agent_header->hvalue, "") == 0) {
		bad_actor_event->user_agent =
			util_duplicate_string(BAD_ACTOR_NOT_FOUND);
	} else if (user_agent_header != NULL &&
		   user_agent_header->hvalue != NULL) {
		bad_actor_event->user_agent =
			util_duplicate_string(user_agent_header->hvalue);
	} else {
		bad_actor_event->user_agent =
			util_duplicate_string(BAD_ACTOR_NOT_FOUND);
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr,
			"Bad Actor is:\n"
			"Event Timestamp: %s\n"
			"Event UUID: %s\n"
			"SIP Message: %s\n"
			"Source IP: %s\n"
			"Called Number: %s\n"
			"SIP Method: %s\n"
			"Transport Type: %s\n"
			"User Agent: %s\n"
			"Collected Method: %s\n"
			"Created by Node Id: %s\n",
			bad_actor_event->event_timestamp,
			bad_actor_event->event_uuid,
			bad_actor_event->sip_message,
			bad_actor_event->source_ip,
			bad_actor_event->called_number, bad_actor_event->method,
			bad_actor_event->transport_type,
			bad_actor_event->user_agent,
			bad_actor_event->collected_method,
			bad_actor_event->created_by_node_id);
	}
	osip_message_free(parsed_sip_message);

	return EXIT_SUCCESS;
}
