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
#ifndef SENTRYPEER_HTTP_COMMON_H
#define SENTRYPEER_HTTP_COMMON_H 1

#include <stdbool.h>
#include <microhttpd.h>

#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_JSON "application/json"
#define STATUS_OK_JSON "{\"status\": \"OK\"}"
#define NOT_FOUND_ERROR_JSON                                                   \
	"{\"error\": \"The requested resource could not be found.\"}"
#define BAD_DATA_JSON "{\"error\": \"The request data was invalid\"}"
#define NOT_FOUND_BAD_ACTOR_JSON "{\"message\": \"No bad actor found\"}"
#define NOT_FOUND_BAD_ACTORS_JSON "{\"message\": \"No bad actors found\"}"
#define NOT_FOUND_PHONE_NUMBER_JSON "{\"message\": \"No phone number found\"}"
#define NOT_FOUND_PHONE_NUMBERS_JSON "{\"message\": \"No phone numbers found\"}"

void log_http_client_ip(const char *url, struct MHD_Connection *connection);
bool json_is_requested(struct MHD_Connection *connection);

int finalise_response(struct MHD_Connection *connection, const char *reply_data,
		      const char *content_type, int status_code,
		      bool free_reply_data);

#endif //SENTRYPEER_HTTP_COMMON_H
