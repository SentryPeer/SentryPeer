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

#ifndef SENTRYPEER_TEST_BAD_ACTOR_H
#define SENTRYPEER_TEST_BAD_ACTOR_H 1

#include "../../src/bad_actor.h"

void test_bad_actor(void **state);
void test_bad_actors(void **state);
bad_actor *test_bad_actor_event_new(void);

#endif //SENTRYPEER_TEST_BAD_ACTOR_H
