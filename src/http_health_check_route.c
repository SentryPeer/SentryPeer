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

#include "http_common.h"
#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

int health_check_route(struct MHD_Connection *connection)
{
	const char *reply_to_get = 0;
	const char *html_text =
		"<html><body><h1>Hello from SentryPeer!</h1><h2>All is well!</h2></body></html>";
	const char *content_type = 0;

	if (json_is_requested(connection)) {
		json_t *api_reply_to_get_json =
			json_pack("{s:s, s:s, s:s}", "status", "OK", "message",
				  "Hello from SentryPeer!", "version",
				  PACKAGE_VERSION);
		reply_to_get =
			json_dumps(api_reply_to_get_json, JSON_INDENT(2));
		content_type = CONTENT_TYPE_JSON;
		// Free the json object
		json_decref(api_reply_to_get_json);
	} else {
		content_type = CONTENT_TYPE_HTML;
		reply_to_get = html_text;
	}

	return finalise_response(connection, reply_to_get, content_type,
				 MHD_HTTP_OK);
}
