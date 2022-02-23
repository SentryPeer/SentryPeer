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

// Bring in getaddrinfo and others as -std=c18 bins them off
#define _GNU_SOURCE

#ifdef __APPLE__
#define __APPLE_USE_RFC_3542
#endif

#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <config.h>

#include "conf.h"
#include "sip_daemon.h"
#include "sip_parser.h"
#include "database.h"
#include "json_logger.h"

#define PACKET_BUFFER_SIZE 1024

void *sip_daemon_thread_start(void *arg)
{
	sip_daemon_init((sentrypeer_config *)arg);
	return NULL;
}

int sip_daemon_run(sentrypeer_config *config)
{
	pthread_t sip_daemon_thread = 0;
	const char *sip_daemon_thread_name = "sip_daemon";

	if (pthread_create(&sip_daemon_thread, NULL, sip_daemon_thread_start,
			   (void *)config) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to create SIP daemon thread.\n");
		return EXIT_FAILURE;
	}
#ifdef __APPLE__
	if (pthread_setname_np(sip_daemon_thread_name) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to set SIP daemon thread name.\n");
		return EXIT_FAILURE;
	}
#else
	if (pthread_setname_np(sip_daemon_thread, sip_daemon_thread_name) !=
	    EXIT_SUCCESS) {
		fprintf(stderr, "Failed to set SIP daemon thread name.\n");
		return EXIT_FAILURE;
	}
#endif
	config->sip_daemon_thread = sip_daemon_thread;

	return EXIT_SUCCESS;
}

int sip_daemon_stop(sentrypeer_config const *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping sip daemon...\n");
	}

	if (pthread_cancel(config->sip_daemon_thread) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to cancel SIP daemon thread.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

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
 * recvmsg();
 * sip_parse_request(); etc
 * send();
 *
 */

/*
 * sip_daemon_init
 *
 * TODO: Implement proper logging? What do we need to log?
 */

int sip_daemon_init(sentrypeer_config const *config)
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
	int enable = 1;
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, (void *)&enable,
		       sizeof(enable)) < 0) {
		perror("setsockopt() failed.");
		CLOSESOCKET(socket_listen);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	int optname = 0;
	int protocol = IPPROTO_IP;
#ifdef HAVE_IP_PKTINFO
	if (bind_address->ai_family == AF_INET) {
		optname = IP_PKTINFO;
	} else if (bind_address->ai_family == AF_INET6) {
		optname = IPV6_RECVPKTINFO;
		protocol = IPPROTO_IPV6;
	}
#elif defined(HAVE_IP_RECVDSTADDR)
	optname = IP_RECVDSTADDR;
#endif

	// TODO: move to a function if a new setsockopt() is needed
	enable = 1;
	if (setsockopt(socket_listen, protocol, optname, &enable,
		       sizeof(enable)) < 0) {
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
		perror("bind() failed");
		CLOSESOCKET(socket_listen);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}
	freeaddrinfo(bind_address); // Not needed anymore

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen, &master);
	SOCKET max_socket = socket_listen;

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Listening for incoming connections...\n");

		if (config->sip_responsive_mode) {
			fprintf(stderr,
				"SIP responsive mode enabled. Will reply to SIP probes...\n");
		}
	}

	// TODO: Switch to libevent/0MQ/epoll etc. later if needed
	while (1) {
		fd_set reads;
		reads = master;
		if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
			perror("select() failed.");
			return EXIT_FAILURE;
		}

		if (FD_ISSET(socket_listen, &reads)) {
			struct sockaddr_storage client_address;
			socklen_t client_len = sizeof(client_address);

			char read_packet_buf[PACKET_BUFFER_SIZE] = { 0 };
			char cmbuf[0x100];

			struct msghdr msg_hdr = {
				.msg_name = &client_address,
				.msg_namelen = sizeof(client_address),
				.msg_control = cmbuf,
				.msg_controllen = sizeof(cmbuf),
			};

			struct iovec iov = {
				.iov_base = read_packet_buf,
				.iov_len = sizeof(read_packet_buf),
			};

			msg_hdr.msg_iov = &iov;
			msg_hdr.msg_iovlen = 1;

			int bytes_received =
				recvmsg(socket_listen, &msg_hdr, 0);

			if (bytes_received < 1) {
				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"Empty packet received.\n");
				}
			}

			// TODO: Clean up
			char *dest_ip_address_buffer = 0;
#ifdef HAVE_IP_PKTINFO
			for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg_hdr);
			     cmsg != NULL; cmsg = CMSG_NXTHDR(&msg_hdr, cmsg)) {
				if (cmsg->cmsg_level != IPPROTO_IP ||
				    cmsg->cmsg_type != IP_PKTINFO) {
					continue;
				}
				struct in_pktinfo const *pi =
					(struct in_pktinfo *)CMSG_DATA(cmsg);

				dest_ip_address_buffer = util_duplicate_string(
					inet_ntoa(pi->ipi_spec_dst));

				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"Destination IP address of UDP packet is: %s\n",
						inet_ntoa(pi->ipi_spec_dst));
				}
			}
#elif defined(HAVE_IP_RECVDSTADDR)
			for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg_hdr);
			     cmsg != NULL; cmsg = CMSG_NXTHDR(&msg_hdr, cmsg)) {
				if (cmsg->cmsg_level != IPPROTO_IP ||
				    cmsg->cmsg_type != IP_RECVDSTADDR) {
					continue;
				}
				struct in_addr *in =
					(struct in_addr *)CMSG_DATA(cmsg);
				if (config->debug_mode ||
				    config->verbose_mode) {
					fprintf(stderr,
						"Destination IP address of UDP packet is: %s\n",
						inet_ntoa(in->ipi_spec_dst));
				}
			}
#endif
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
#if INTPTR_MAX == INT64_MAX
				fprintf(stderr,
					"read_packet_buf size is: %lu: \n",
					sizeof(read_packet_buf));
				fprintf(stderr,
					"read_packet_buf length is: %lu: \n",
					strnlen(read_packet_buf,
						PACKET_BUFFER_SIZE));
#endif
				fprintf(stderr,
					"bytes_received size is: %d: \n\n",
					bytes_received);
			}

			// TODO: update once with have TCP and TLS
			char transport_type[] = "UDP";
			char collected_method[11] =
				"passive"; // size is responsive + 1
			if (config->sip_responsive_mode == true)
				strcpy(collected_method, "responsive");

			bad_actor *bad_actor_event = bad_actor_new(
				0,
				util_duplicate_string(client_ip_address_buffer),
				dest_ip_address_buffer, 0, 0,
				util_duplicate_string(transport_type), 0,
				util_duplicate_string(collected_method), 0);

			if (bytes_received > 0) {
				if ((sip_message_parser(
					    read_packet_buf, bytes_received,
					    bad_actor_event, config)) !=
				    EXIT_SUCCESS) {
					if (config->debug_mode ||
					    config->verbose_mode) {
						fprintf(stderr,
							"Parsing this SIP packet failed.\n");
					}
				}
			}

			if (config->syslog_mode) {
				syslog(LOG_NOTICE,
				       "Source IP: %s, Method: %s, Agent: %s\n",
				       bad_actor_event->source_ip,
				       bad_actor_event->method,
				       bad_actor_event->user_agent);
			}

			if (config->json_log_mode &&
			    (json_log_bad_actor(config, bad_actor_event) !=
			     EXIT_SUCCESS)) {
				fprintf(stderr,
					"Saving bad_actor json to %s failed.\n",
					config->json_log_file);
			}

			if (db_insert_bad_actor(bad_actor_event, config) !=
			    EXIT_SUCCESS) {
				fprintf(stderr,
					"Saving bad actor to db failed\n");
			}
			bad_actor_destroy(&bad_actor_event);

			if (config->sip_responsive_mode) {
				// TODO Create reply headers with libosip2. Bad
				// Actors don't see to care we're always replying
				// with 200 OK/non-compliant SIP :-)
				char SIP_200_OK[] =
					"SIP/2.0 200 OK\n"
					"Via: SIP/2.0/UDP 127.0.0.1:56940\n"
					"Call-ID: 1179563087@127.0.0.1\n"
					"From: <sip:sipsak@127.0.0.1>;tag=464eb44f\n"
					"To: <sip:asterisk@127.0.0.1>;tag=z9hG4bK.1c882828\n"
					"CSeq: 1 OPTIONS\n"
					"Accept: application/sdp, application/dialog-info+xml, application/simple-message-summary, application/xpidf+xml, application/cpim-pidf+xml, application/pidf+xml, application/pidf+xml, application/dialog-info+xml, application/simple-message-summary, message/sipfrag;version=2.0\n"
					"Allow: OPTIONS, SUBSCRIBE, NOTIFY, PUBLISH, INVITE, ACK, BYE, CANCEL, UPDATE, PRACK, REGISTER, REFER, MESSAGE\n"
					"Supported: 100rel, timer, replaces, norefersub\n"
					"Accept-Encoding: text/plain\n"
					"Accept-Language: en\n"
					"Server: FPBX-14.0.16.11(14.7.8)\n"
					"Content-Length:  0";

				long bytes_sent = sendto(
					socket_listen, SIP_200_OK,
					sizeof(SIP_200_OK), 0,
					(struct sockaddr *)&client_address,
					client_len);
				if (bytes_sent < 1) {
					if (config->debug_mode ||
					    config->verbose_mode) {
						perror("sendto() failed.");
					}
				}
			}
		}
	}
	CLOSESOCKET(socket_listen);
}
