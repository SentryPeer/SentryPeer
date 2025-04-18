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
#include <assert.h>
#include <errno.h>

#include "conf.h"
#include "sip_daemon.h"
#include "sip_message_event.h"
#include "sip_parser.h"

#if HAVE_OPENDHT_C != 0
#include "peer_to_peer_dht.h"
#endif // HAVE_OPENDHT_C

#if HAVE_RUST != 0
#include "sentrypeer_rust.h"
#endif // HAVE_RUST

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

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

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
		// Error is?
		fprintf(stderr, "Error is: %d\n", errno);
		// Error description?
		fprintf(stderr, "Error description is: %s\n", strerror(errno));

		return EXIT_FAILURE;
	}
#endif
	config->sip_daemon_thread = sip_daemon_thread;

	return EXIT_SUCCESS;
}

#if HAVE_RUST != 0
int shutdown_sip_server(sentrypeer_config const *config)
{
	if (shutdown_sip(config) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to shutdown TLS listener.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
#endif // HAVE_RUST

int sip_daemon_stop(sentrypeer_config const *config)
{
    // default if no Rust or set via config file/CLI/ENV
	if (config->new_mode == false) {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Stopping sip daemon...\n");
		}

		if (pthread_cancel(config->sip_daemon_thread) != EXIT_SUCCESS) {
			fprintf(stderr,
				"Failed to cancel SIP daemon thread.\n");
			return EXIT_FAILURE;
		}

		if (pthread_join(config->sip_daemon_thread, NULL) !=
		    EXIT_SUCCESS) {
			fprintf(stderr, "Failed to join SIP daemon thread.\n");
			return EXIT_FAILURE;
		}
	}

	// Shutdown our Rust listeners
#if HAVE_RUST != 0
	if (config->new_mode == true) {
		if (shutdown_sip_server(config) != EXIT_SUCCESS) {
			fprintf(stderr, "Failed to shutdown TLS listener.\n");
			return EXIT_FAILURE;
		}
	}
#endif // HAVE_RUST

	return EXIT_SUCCESS;
}

int sip_log_event(sentrypeer_config *config, const sip_message_event *sip_event)
{
	char collected_method[11] = "passive"; // size is responsive + 1
	if (config->sip_responsive_mode == true)
		strcpy(collected_method, "responsive");

	bad_actor *bad_actor_event = bad_actor_new(
		0, util_duplicate_string(sip_event->client_ip_addr_str),
		util_duplicate_string(sip_event->dest_ip_addr_str), 0, 0,
		util_duplicate_string(sip_event->transport_type), 0,
		util_duplicate_string(collected_method), config->node_id);
	assert(bad_actor_event);

	if (sip_event->packet_len > 0) {
		if ((sip_message_parser(sip_event->packet,
					sip_event->packet_len, bad_actor_event,
					config)) != EXIT_SUCCESS) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Parsing this SIP packet failed.\n");
			}
			bad_actor_destroy(&bad_actor_event);
			return EXIT_FAILURE;
		}
	}

	if (bad_actor_log(config, bad_actor_event) != EXIT_SUCCESS) {
		fprintf(stderr, "Logging bad_actor failed.\n");
		bad_actor_destroy(&bad_actor_event);
		return EXIT_FAILURE;
	}

// Put on DHT last
#if HAVE_OPENDHT_C != 0
	if (config->p2p_dht_mode &&
	    peer_to_peer_dht_save(config, bad_actor_event) != EXIT_SUCCESS) {
		fprintf(stderr,
			"Error saving bad_actor to peer_to_peer_dht.\n");
		bad_actor_destroy(&bad_actor_event);
		return EXIT_FAILURE;
	}
#endif // HAVE_OPENDHT_C

	bad_actor_destroy(&bad_actor_event);
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "SIP packet logged.\n");
	}

	return EXIT_SUCCESS;
}

int sip_send_reply(sentrypeer_config const *config,
		   sip_message_event const *sip_event)
{
	// TODO Create reply headers with libosip2. Bad
	// Actors don't seem to care we're always replying
	// with 200 OK/non-compliant SIP :-)
	char SIP_200_OK[] =
		"SIP/2.0 200 OK\r\n"
		"Via: SIP/2.0/UDP 127.0.0.1:56940\r\n"
		"Call-ID: 1179563087@127.0.0.1\r\n"
		"From: <sip:sipsak@127.0.0.1>;tag=464eb44f\r\n"
		"To: <sip:asterisk@127.0.0.1>;tag=z9hG4bK.1c882828\r\n"
		"CSeq: 1 OPTIONS\r\n"
		"Accept: application/sdp, application/dialog-info+xml, application/simple-message-summary, application/xpidf+xml, application/cpim-pidf+xml, application/pidf+xml, application/pidf+xml, application/dialog-info+xml, application/simple-message-summary, message/sipfrag;version=2.0\r\n"
		"Allow: OPTIONS, SUBSCRIBE, NOTIFY, PUBLISH, INVITE, ACK, BYE, CANCEL, UPDATE, PRACK, REGISTER, REFER, MESSAGE\r\n"
		"Supported: 100rel, timer, replaces, norefersub\r\n"
		"Accept-Encoding: text/plain\r\n"
		"Accept-Language: en\r\n"
		"Server: FPBX-16.0.33(18.13.0)\r\n"
		"Content-Length:  0\r\n";

	long bytes_sent =
		sendto(sip_event->socket, SIP_200_OK, sizeof(SIP_200_OK), 0,
		       sip_event->client_ip_addr, sip_event->client_addr_len);
	if (bytes_sent < 1) {
		if (config->debug_mode || config->verbose_mode) {
			perror("sendto() failed.");
			return EXIT_FAILURE;
		}
	}
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "SIP reply sent.\n");
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

int sip_daemon_init(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Configuring local address...\n");
	}
	struct addrinfo gai_hints;
	memset(&gai_hints, 0, sizeof(gai_hints));
	gai_hints.ai_family = AF_INET;
	gai_hints.ai_socktype = SOCK_DGRAM; // UDP
	gai_hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address = 0;
	int gai = getaddrinfo(0, SIP_DAEMON_PORT, &gai_hints, &bind_address);
	if (gai != EXIT_SUCCESS) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Creating sockets...\n");
	}
	SOCKET socket_listen_udp;
	socket_listen_udp =
		socket(bind_address->ai_family, bind_address->ai_socktype,
		       bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen_udp)) {
		perror("UDP socket() failed.");
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}
	freeaddrinfo(bind_address);

	gai_hints.ai_socktype = SOCK_STREAM; // TCP
	gai = getaddrinfo(0, SIP_DAEMON_PORT, &gai_hints, &bind_address);
	if (gai != EXIT_SUCCESS) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
		CLOSESOCKET(socket_listen_udp);
		return EXIT_FAILURE;
	}

	SOCKET socket_listen_tcp;
	socket_listen_tcp =
		socket(bind_address->ai_family, bind_address->ai_socktype,
		       bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen_tcp)) {
		perror("TCP socket() failed.");
		CLOSESOCKET(socket_listen_udp);
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
	if (setsockopt(socket_listen_udp, SOL_SOCKET, SO_REUSEADDR,
		       (void *)&enable, sizeof(enable)) != EXIT_SUCCESS) {
		perror("UDP setsockopt() failed.");
		CLOSESOCKET(socket_listen_udp);
		CLOSESOCKET(socket_listen_tcp);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	enable = 1;
	if (setsockopt(socket_listen_tcp, SOL_SOCKET, SO_REUSEADDR,
		       (void *)&enable, sizeof(enable)) != EXIT_SUCCESS) {
		perror("TCP setsockopt() failed.");
		CLOSESOCKET(socket_listen_tcp);
		CLOSESOCKET(socket_listen_udp);
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
	if (setsockopt(socket_listen_udp, protocol, optname, &enable,
		       sizeof(enable)) != EXIT_SUCCESS) {
		perror("UDP setsockopt() failed.");
		CLOSESOCKET(socket_listen_udp);
		CLOSESOCKET(socket_listen_tcp);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Binding sockets to local address...\n");
	}
	if (bind(socket_listen_udp, bind_address->ai_addr,
		 bind_address->ai_addrlen) != EXIT_SUCCESS) {
		perror("UDP bind() failed");
		CLOSESOCKET(socket_listen_udp);
		CLOSESOCKET(socket_listen_tcp);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Listening for incoming UDP connections...\n");

		if (config->sip_responsive_mode) {
			fprintf(stderr,
				"SIP responsive mode enabled. Will reply to SIP probes...\n");
		}
	}

	if (bind(socket_listen_tcp, bind_address->ai_addr,
		 bind_address->ai_addrlen) != EXIT_SUCCESS) {
		perror("TCP bind() failed");
		CLOSESOCKET(socket_listen_udp);
		CLOSESOCKET(socket_listen_tcp);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Listening for incoming TCP connections...\n");
	}
	if (listen(socket_listen_tcp, 10) != EXIT_SUCCESS) {
		perror("TCP listen() failed");
		CLOSESOCKET(socket_listen_tcp);
		CLOSESOCKET(socket_listen_udp);
		freeaddrinfo(bind_address);
		return EXIT_FAILURE;
	}
	freeaddrinfo(bind_address);

	fd_set master;
	FD_ZERO(&master);
	FD_SET(socket_listen_udp, &master);
	FD_SET(socket_listen_tcp, &master);
	SOCKET max_socket = max_int(socket_listen_tcp, socket_listen_udp);

	// TODO: Switch to libevent/0MQ/epoll etc. later if needed
	while (1) {
		fd_set reads;
		reads = master;
		char *dest_ip_address_buffer = 0;
		if (select(max_socket + 1, &reads, 0, 0, 0) < EXIT_SUCCESS) {
			perror("select() failed.");
			return EXIT_FAILURE;
		}

		for (SOCKET i = 1; i <= max_socket; ++i) {
			if (FD_ISSET(i, &reads)) {
				char tcp_client_ip_address_buffer[100];
				if (i == socket_listen_tcp) {
					struct sockaddr_storage
						tcp_client_address;
					socklen_t tcp_client_len =
						sizeof(tcp_client_address);

					SOCKET tcp_socket_client = accept(
						socket_listen_tcp,
						(struct sockaddr
							 *)&tcp_client_address,
						&tcp_client_len);
					if (!ISVALIDSOCKET(tcp_socket_client)) {
						perror("TCP accept() failed.");
						CLOSESOCKET(tcp_socket_client);
						continue;
					}

					FD_SET(tcp_socket_client, &master);
					if (tcp_socket_client > max_socket) {
						max_socket = tcp_socket_client;
					}

					char client_send_port_buffer[100];
					if (getnameinfo(
						    ((struct sockaddr
							      *)&tcp_client_address),
						    tcp_client_len,
						    tcp_client_ip_address_buffer,
						    sizeof(tcp_client_ip_address_buffer),
						    client_send_port_buffer,
						    sizeof(client_send_port_buffer),
						    NI_NUMERICHOST |
							    NI_NUMERICSERV) !=
					    EXIT_SUCCESS) {
						perror("getnameinfo() failed.");
						return EXIT_FAILURE;
					}

					if (config->debug_mode ||
					    config->verbose_mode) {
						fprintf(stderr,
							"Accepted TCP connection from %s\n",
							tcp_client_ip_address_buffer);
					}
				}

				// Process a connected TCP client
				if ((i != socket_listen_udp) &&
				    (i != socket_listen_tcp)) {
					char read_packet_buf
						[PACKET_BUFFER_SIZE] = { 0 };
					int bytes_received =
						recv(i, read_packet_buf,
						     PACKET_BUFFER_SIZE, 0);
					if (bytes_received < 1) {
						if (config->debug_mode ||
						    config->verbose_mode) {
							fprintf(stderr,
								"Empty TCP packet received.\n");
						}
						FD_CLR(i, &master);
						CLOSESOCKET(i);
						continue;
					}

					if (config->debug_mode ||
					    config->verbose_mode) {
						time_t timestamp;
						time(&timestamp);
						fprintf(stderr,
							"epochtime: %ld\nReceived (%d bytes): %.*s\n",
							timestamp,
							bytes_received,
							bytes_received,
							read_packet_buf);

						fprintf(stderr,
							"Received TCP packet from %s\n",
							tcp_client_ip_address_buffer);
					}

					struct sockaddr_in destination_address;
					socklen_t destination_address_len =
						sizeof(destination_address);
					memset(&destination_address, 0,
					       destination_address_len);
					destination_address.sin_family =
						AF_INET;
					if (getsockname(
						    i,
						    (struct sockaddr
							     *)&destination_address,
						    &destination_address_len) ==
					    EXIT_SUCCESS) {
						dest_ip_address_buffer =
							util_duplicate_string(inet_ntoa(
								destination_address
									.sin_addr));

						if (config->debug_mode ||
						    config->verbose_mode) {
							fprintf(stderr,
								"Destination IP address of TCP packet is: %s\n",
								inet_ntoa(
									destination_address
										.sin_addr));
						}
					}
					assert(dest_ip_address_buffer);
					socklen_t tcp_client_len = sizeof(i);

					char transport_type[] = "TCP";
					sip_message_event *sip_event = sip_message_event_new(

						util_duplicate_string(
							read_packet_buf),
						bytes_received, i,
						util_duplicate_string(
							transport_type),
						(struct sockaddr *)&i,
						util_duplicate_string(
							tcp_client_ip_address_buffer),

						tcp_client_len,

						dest_ip_address_buffer);
					assert(sip_event);
					if (sip_log_event(config, sip_event) !=
					    EXIT_SUCCESS) {
						fprintf(stderr,
							"Failed to log SIP TCP event.\n");
						continue;
					}

					if (config->sip_responsive_mode) {
						if (sip_send_reply(config,
								   sip_event) !=
						    EXIT_SUCCESS) {
							fprintf(stderr,
								"Error sending SIP reply.\n");
							continue;
						}
					}
					sip_message_event_destroy(&sip_event);
				}

				if (FD_ISSET(socket_listen_udp, &reads)) {
					struct sockaddr_storage client_address;
					socklen_t client_len =
						sizeof(client_address);

					char read_packet_buf
						[PACKET_BUFFER_SIZE] = { 0 };
					char cmbuf[0x100];

					struct msghdr msg_hdr = {
						.msg_name = &client_address,
						.msg_namelen =
							sizeof(client_address),
						.msg_control = cmbuf,
						.msg_controllen = sizeof(cmbuf),
					};

					struct iovec iov = {
						.iov_base = read_packet_buf,
						.iov_len =
							sizeof(read_packet_buf),
					};

					msg_hdr.msg_iov = &iov;
					msg_hdr.msg_iovlen = 1;

					int bytes_received = recvmsg(
						socket_listen_udp, &msg_hdr, 0);

					if (bytes_received < 1) {
						if (config->debug_mode ||
						    config->verbose_mode) {
							fprintf(stderr,
								"Empty UDP packet received.\n");
						}
					}

					// TODO: Clean up
#ifdef HAVE_IP_PKTINFO
					for (struct cmsghdr *cmsg =
						     CMSG_FIRSTHDR(&msg_hdr);
					     cmsg != NULL;
					     cmsg = CMSG_NXTHDR(&msg_hdr,
								cmsg)) {
						if (cmsg->cmsg_level !=
							    IPPROTO_IP ||
						    cmsg->cmsg_type !=
							    IP_PKTINFO) {
							continue;
						}
						struct in_pktinfo const *pi =
							(struct in_pktinfo *)
								CMSG_DATA(cmsg);

						dest_ip_address_buffer =
							util_duplicate_string(inet_ntoa(
								pi->ipi_spec_dst));

						if (config->debug_mode ||
						    config->verbose_mode) {
							fprintf(stderr,
								"Destination IP address of UDP packet is: %s\n",
								inet_ntoa(
									pi->ipi_spec_dst));
						}
					}
#elif defined(HAVE_IP_RECVDSTADDR)
					for (struct cmsghdr *cmsg =
						     CMSG_FIRSTHDR(&msg_hdr);
					     cmsg != NULL;
					     cmsg = CMSG_NXTHDR(&msg_hdr,
								cmsg)) {
						if (cmsg->cmsg_level !=
							    IPPROTO_IP ||
						    cmsg->cmsg_type !=
							    IP_RECVDSTADDR) {
							continue;
						}
						struct in_addr *in =
							(struct in_addr *)
								CMSG_DATA(cmsg);
						if (config->debug_mode ||
						    config->verbose_mode) {
							fprintf(stderr,
								"Destination IP address of UDP packet is: %s\n",
								inet_ntoa(
									in->ipi_spec_dst));
						}
					}
#endif
					// Format timestamp like ngrep does
					// https://github.com/jpr5/ngrep/blob/2a9603bc67dface9606a658da45e1f5c65170444/ngrep.c#L1247
					if (config->debug_mode ||
					    config->verbose_mode) {
						time_t timestamp;
						time(&timestamp);
						fprintf(stderr,
							"epochtime: %ld\nReceived (%d bytes): %.*s\n",
							timestamp,
							bytes_received,
							bytes_received,
							read_packet_buf);
					}

					char udp_client_ip_address_buffer[100];
					char udp_client_send_port_buffer[100];
					if (getnameinfo(
						    ((struct sockaddr
							      *)&client_address),
						    client_len,
						    udp_client_ip_address_buffer,
						    sizeof(udp_client_ip_address_buffer),
						    udp_client_send_port_buffer,
						    sizeof(udp_client_send_port_buffer),
						    NI_NUMERICHOST |
							    NI_NUMERICSERV) !=
					    EXIT_SUCCESS) {
						perror("getnameinfo() failed.");
						return EXIT_FAILURE;
					}

					if (config->debug_mode ||
					    config->verbose_mode) {
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
					char transport_type[] = "UDP";

					sip_message_event *sip_event = sip_message_event_new(
						util_duplicate_string(
							read_packet_buf),
						bytes_received,
						socket_listen_udp,
						util_duplicate_string(
							transport_type),
						(struct sockaddr
							 *)&client_address,

						util_duplicate_string(
							udp_client_ip_address_buffer),
						client_len,

						dest_ip_address_buffer);
					assert(sip_event);

					if (sip_log_event(config, sip_event) !=
					    EXIT_SUCCESS) {
						fprintf(stderr,
							"Failed to log SIP UDP event.\n");
						continue;
					}

					if (config->sip_responsive_mode) {
						if (sip_send_reply(config,
								   sip_event) !=
						    EXIT_SUCCESS) {
							fprintf(stderr,
								"Error sending SIP reply.\n");
							continue;
						}
					}
					sip_message_event_destroy(&sip_event);
				}
			}
		}
	}
	CLOSESOCKET(socket_listen_udp);
	CLOSESOCKET(socket_listen_tcp);
}
