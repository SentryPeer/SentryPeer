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

#ifndef SENTRYPEER_TEST_SENTRYPEER_RUST_H
#define SENTRYPEER_TEST_SENTRYPEER_RUST_H 1

#include <stdbool.h>

void test_sentrypeer_rust(void **state);
void display_rust(void);
int32_t return_exit_status(bool success);
char *return_string(void);
void free_string(char *s);

#endif //SENTRYPEER_TEST_SENTRYPEER_RUST_H
