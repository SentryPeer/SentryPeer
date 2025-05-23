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

#ifndef SENTRYPEER_TEST_SENTRYPEER_RUST_H
#define SENTRYPEER_TEST_SENTRYPEER_RUST_H 1

#include <stdbool.h>
#include "test_bad_actor.h"

void test_sentrypeer_rust(void **state);
void display_rust(void);
int32_t return_exit_status(bool success);
// https://stackoverflow.com/a/1789851
int32_t callback_from_c(int32_t (*callback)(bool), bool success);
char *return_string(void);
void free_string(char *s);
bad_actor *return_bad_actor_new(const char *sip_message, const char *source_ip,
				const char *destination_ip,
				const char *called_number, const char *method,
				const char *transport_type,
				const char *user_agent,
				const char *collected_method,
				const char *created_by_node_id);

#endif //SENTRYPEER_TEST_SENTRYPEER_RUST_H
