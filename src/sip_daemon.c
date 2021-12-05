/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

// Bring in getaddrinfo and others as -std=c18 bins them off
#define _GNU_SOURCE

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include "conf.h"
#include "sip_daemon.h"
#include "sip_parser.h"
#include "database.h"

#define PACKET_BUFFER_SIZE 1024

/*
 * Hands-On Network Programming with C, page 117
 * UDP Server (move to libevent and/or 0MQ Routers/zproto/zproject/Zyre:
 *
 * getaddrinfo();
 * socket();
 * bind();
 * select();
 *
 * Has socket() input? Yes, move on. No, go back to select().
 *
 * recvfrom();
 * sip_parse_request(); etc
 * send();
 *
 */

/*
 * sip_daemon_init
 *
 * TODO: Implement proper logging? What do we need to log?
 */

int sip_daemon_init(struct sentrypeer_config const *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Configuring local address...\n");
	}
	struct addrinfo gai_hints;
	memset(&gai_hints, 0, sizeof(gai_hints));
	gai_hints.ai_family = AF_INET;
	gai_hints.ai_socktype = SOCK_DGRAM; // UDP
	gai_hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;
	int gai = getaddrinfo(0, SIP_DAEMON_PORT, &gai_hints, &bind_address);
	if (gai != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Creating socket...\n");
	}
	SOCKET socket_listen;
	socket_listen =
		socket(bind_address->ai_family, bind_address->ai_socktype,
		       bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen)) {
		perror("socket() failed.");
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	/* The failure of the bind() call can be prevented by setting
	 * the SO_REUSEADDR flag on the server socket before calling bind().
	 *
	 * Eliminates "ERROR on binding: Address already in use" error.
	 * See Hands-On Network Programming with C, page 374 of PDF.
	 *
	 * We also need the destination IP address, so we need to set
	 * IP_PKTINFO or IP_ORIGDSTADDR on Linux and IP_RECVDSTADDR on BSD.
	 *
	 */
	int yes = 1;
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, (void *)&yes,
		       sizeof(yes)) < 0) {
		fprintf(stderr, "setsockopt() failed. (%d)\n",
			GETSOCKETERRNO());
		perror("setsockopt() failed.");
		CLOSESOCKET(socket_listen);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Binding socket to local address...\n");
	}
	if (bind(socket_listen, bind_address->ai_addr,
		 bind_address->ai_addrlen)) {
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		perror("bind() failed");
		CLOSESOCKET(socket_listen);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}
	freeaddrinfo(bind_address); // Not needed anymore

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Setting database error log callback...\n");
	}
	if (db_set_error_log_callback() != EXIT_SUCCESS) {
		fprintf(stderr, "Couldn't set database error log callback\n");
		CLOSESOCKET(socket_listen);
		return EXIT_FAILURE;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Listening for incoming connections...\n");
	}

	// TODO: Switch to libevent/0MQ/epoll etc. later if needed
	while (1) {
		fd_set reads;
		reads = master;
		if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
			fprintf(stderr, "select() failed. (%d)\n",
				GETSOCKETERRNO());
			perror("select() failed.");
			return EXIT_FAILURE;
		}

		if (FD_ISSET(socket_listen, &reads)) {
			struct sockaddr_storage client_address;
			socklen_t client_len = sizeof(client_address);

			char read_packet_buf[PACKET_BUFFER_SIZE];
			int bytes_received =
				recvfrom(socket_listen, read_packet_buf,
					 PACKET_BUFFER_SIZE, 0,
					 (struct sockaddr *)&client_address,
					 &client_len);
			if (bytes_received < 1) {
				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"connection closed. (%d)\n",
						GETSOCKETERRNO());
					perror("recvfrom() failed.");
				}
				return EXIT_FAILURE;
			}

			// Format timestamp like ngrep does
			// https://github.com/jpr5/ngrep/blob/2a9603bc67dface9606a658da45e1f5c65170444/ngrep.c#L1247
			if (config->debug_mode || config->verbose_mode) {
				time_t timestamp;
				time(&timestamp);
				fprintf(stderr,
					"epochtime: %ld\nReceived (%d bytes): %.*s\n",
					timestamp, bytes_received,
					bytes_received, read_packet_buf);
			}

			char client_ip_address_buffer[100];
			char client_send_port_buffer[100];
			if (getnameinfo(((struct sockaddr *)&client_address),
					client_len, client_ip_address_buffer,
					sizeof(client_ip_address_buffer),
					client_send_port_buffer,
					sizeof(client_send_port_buffer),
					NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
				perror("getnameinfo() failed.");
				return EXIT_FAILURE;
			}

			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"read_packet_buf size is: %lu: \n",
					sizeof(read_packet_buf));
				fprintf(stderr,
					"read_packet_buf length is: %lu: \n",
					strnlen(read_packet_buf,
						PACKET_BUFFER_SIZE));
				fprintf(stderr,
					"bytes_received size is: %d: \n\n",
					bytes_received);
			}

			// TODO: update once with have TCP and TLS
			char transport_type[] = "UDP";
			char collected_method[] = "passive";
			bad_actor *bad_actor_event =
				bad_actor_new(0, client_ip_address_buffer, 0, 0,
					      transport_type, 0,
					      collected_method, 0);

			if ((sip_message_parser(read_packet_buf, bytes_received,
						bad_actor_event, config)) !=
			    EXIT_SUCCESS) {
				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"Parsing this SIP packet failed.\n");
				}
			}

			if (config->syslog_mode) {
				syslog(LOG_NOTICE,
				       "Source IP: %s, Method: %s, Agent: %s\n",
				       bad_actor_event->source_ip,
				       bad_actor_event->method,
				       bad_actor_event->user_agent);
			}

			if (db_insert_bad_actor(bad_actor_event, config) !=
			    EXIT_SUCCESS) {
				fprintf(stderr,
					"Saving bad actor to db failed\n");
			}
			bad_actor_destroy(&bad_actor_event);
		}
	}
	CLOSESOCKET(socket_listen);
}
