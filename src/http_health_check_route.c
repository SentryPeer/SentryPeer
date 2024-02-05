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

#include "http_common.h"
#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

int health_check_route(struct MHD_Connection *connection)
{
	if (json_is_requested(connection)) {
		json_t *api_reply_to_get_json =
			json_pack("{s:s, s:s, s:s}", "status", "OK", "message",
				  "Hello from SentryPeer!", "version",
				  PACKAGE_VERSION);
		const char *json_reply_to_get =
			json_dumps(api_reply_to_get_json, JSON_INDENT(2));
		// Free the json object
		json_decref(api_reply_to_get_json);

		return finalise_response(connection, json_reply_to_get,
					 CONTENT_TYPE_JSON, MHD_HTTP_OK, true);
	} else {
		const char *html_text =
			"<html><body><h1>Hello from SentryPeer!</h1><h2>All is well!</h2></body></html>";
		return finalise_response(connection, html_text,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	}
}
