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

#define _GNU_SOURCE
#include "utils.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <jansson.h>
#include <stdbool.h>

char *event_timestamp(char *event_timestamp)
{
	struct timespec timestamp_ts;
	char timestamp_buf[TIMESTAMP_LEN];
	if (clock_gettime(CLOCK_REALTIME, &timestamp_ts) == -1) {
		perror("clock_gettime() failed.");
	}

	assert(timestamp_ts.tv_nsec < 1000000000);

	struct tm time_info;
	localtime_r(&timestamp_ts.tv_sec, &time_info);
	strftime(timestamp_buf, TIMESTAMP_LEN, "%Y-%m-%d %H:%M:%S", &time_info);

	if (snprintf(event_timestamp, TIMESTAMP_LEN, "%s.%06ld", timestamp_buf,
		     timestamp_ts.tv_nsec) < 0) {
		perror("snprintf() failed.");
	}
	assert(event_timestamp);

	return event_timestamp;
}

char *util_duplicate_string(const char *string)
{
	char *duplicate = NULL;
	assert(string);

	duplicate = strndup(string, strlen(string));

	assert(duplicate);
	// Must be freed by caller.
	return duplicate;
}

char *util_copy_string(char *dest, const char *src, size_t dest_len)
{
	assert(src);
	assert(dest);
	assert(dest_len > 0);

	if (strlen(src) > dest_len) {
		return NULL;
	}

	strncpy(dest, src, dest_len);
	assert(dest);
	assert(dest[strlen(dest)] == 0);

	return dest;
}

char *util_uuid_generate_string(char *uuid_string)
{
	assert(uuid_string);

	uuid_t binary_uuid;
	uuid_generate(binary_uuid);
	assert(binary_uuid != NULL);

	uuid_unparse_lower(binary_uuid, uuid_string);

	assert(uuid_string);
	return uuid_string;
}

int valid_ip_address_format(const char *ip_address_to_check)
{
	assert(ip_address_to_check);
	unsigned char buf_not_used[sizeof(struct in6_addr)];

	if (inet_pton(AF_INET, ip_address_to_check, buf_not_used) == 1) {
		return EXIT_SUCCESS;
	} else if (inet_pton(AF_INET6, ip_address_to_check, buf_not_used) ==
		   1) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

char *util_addr_string(const struct sockaddr *addr)
{
	char *s = NULL;
	switch (addr->sa_family) {
	case AF_INET: {
		const struct sockaddr_in *addr_in =
			(const struct sockaddr_in *)addr;
		s = malloc(INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
		break;
	}
	case AF_INET6: {
		const struct sockaddr_in6 *addr_in6 =
			(const struct sockaddr_in6 *)addr;
		s = malloc(INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s,
			  INET6_ADDRSTRLEN);
		break;
	}
	default:
		break;
	}
	return s;
}

bool is_valid_json_key(const json_t *json, const char *key)
{
	const json_t *value = json_object_get(json, key);
	if (value == NULL) {
		return false;
	}
	if (!json_is_string(value)) {
		return false;
	}
	return true;
}

bool is_valid_uuid(const char *uuid_to_check)
{
	uuid_t uuid;
	if (uuid_parse(uuid_to_check, uuid) != 0) {
		return false;
	}
	return true;
}

int max_int(const int x, const int y)
{
	if (x > y) {
		return x;
	} else {
		return y;
	}
}
