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

#include "sip_message_event.h"
#include <assert.h>
#include <stdlib.h>

//  Constructor
sip_message_event *sip_message_event_new(char *packet, size_t packet_len,
					 SOCKET socket, char *transport_type,
					 struct sockaddr *client_ip_addr,
					 char *client_ip_addr_str,
					 size_t client_ip_addr_len,
					 char *dest_ip_addr_str)
{
	sip_message_event *self = malloc(sizeof(sip_message_event));
	assert(self);

	self->packet = packet;
	self->packet_len = packet_len;
	self->socket = socket;
	self->transport_type = transport_type;
	self->client_ip_addr = client_ip_addr;
	self->client_ip_addr_str = client_ip_addr_str;
	self->client_addr_len = client_ip_addr_len;
	self->dest_ip_addr_str = dest_ip_addr_str;

	return self;
}

//  Destructor
void sip_message_event_destroy(sip_message_event **self_ptr)
{
	assert(self_ptr);
	if (*self_ptr) {
		sip_message_event *self = *self_ptr;

		// Modern C by Manning, Takeaway 6.19
		// "6.19 Initialization or assignment with 0 makes a pointer null."
		if (self->packet != 0) {
			free(self->packet);
			self->packet = 0;
		}

		if (self->transport_type != 0) {
			free(self->transport_type);
			self->transport_type = 0;
		}

		if (self->client_ip_addr_str != 0) {
			free(self->client_ip_addr_str);
			self->client_ip_addr_str = 0;
		}

		if (self->dest_ip_addr_str != 0) {
			free(self->dest_ip_addr_str);
			self->dest_ip_addr_str = 0;
		}

		free(self);
		*self_ptr = 0;
	}
}
