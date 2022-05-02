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

#ifndef SENTRYPEER_SIP_MESSAGE_EVENT_H
#define SENTRYPEER_SIP_MESSAGE_EVENT_H 1

#include <stddef.h>

#define SOCKET int

// Modern C - Manning. Chapter 6, Section 6.4:
// Takeaway 6.29
// (Forward-declare a struct within a typedef using the same identifier
// as the tag name.
//

typedef struct sip_message_event sip_message_event;
struct sip_message_event {
	char *packet;
	size_t packet_len;
	SOCKET socket;
	char *transport_type;
	struct sockaddr *client_ip_addr;
	char *client_ip_addr_str;
	size_t client_addr_len;
	char *dest_ip_addr_str;
};

//  Constructor
sip_message_event *sip_message_event_new(char *packet, size_t packet_len,
					 SOCKET socket, char *transport_type,
					 struct sockaddr *client_ip_addr,
					 char *client_ip_addr_str,
					 size_t client_addr_len,
					 char *dest_ip_addr_str);

//  Destructors
void sip_message_event_destroy(sip_message_event **self_ptr);

#endif //SENTRYPEER_SIP_MESSAGE_EVENT_H
