/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "conf.h"
#include "http_daemon.h"

#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define PAGE                                                                   \
	"<html><head><title>SentryPeer API</title>"                            \
	"</head><body>SentryPeer API demo</body></html>"

static enum MHD_Result ahc_echo(void *cls, struct MHD_Connection *connection,
				const char *url, const char *method,
				const char *version, const char *upload_data,
				size_t *upload_data_size, void **ptr)
{
	static int dummy;
	const char *page = cls;
	struct MHD_Response *response;
	int ret;

	if (0 != strcmp(method, "GET"))
		return MHD_NO; /* unexpected method */
	if (&dummy != *ptr) {
		/* The first time only the headers are valid,
         do not respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}
	if (0 != *upload_data_size)
		return MHD_NO; /* upload data in a GET!? */
	*ptr = NULL; /* clear context pointer */
	response = MHD_create_response_from_buffer(strlen(page), (void *)page,
						   MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
}

int http_daemon_init(struct sentrypeer_config const *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting http daemon...\n");
	}

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, 8082, NULL,
				  NULL, &ahc_echo, PAGE, MHD_OPTION_END);
	assert(daemon);
	return EXIT_SUCCESS;

	// MHD_stop_daemon(d);
}
