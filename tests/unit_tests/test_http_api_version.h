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

#ifndef SENTRYPEER_TEST_HTTP_API_VERSION_H
#define SENTRYPEER_TEST_HTTP_API_VERSION_H 1

int test_http_api_health_check_version(void);

typedef struct memory_struct memory_struct;
struct memory_struct {
	char *memory;
	size_t size;
};

#endif //SENTRYPEER_TEST_HTTP_API_VERSION_H
