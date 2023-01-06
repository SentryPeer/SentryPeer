/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2023 Gavin Henry <ghenry@sentrypeer.org> */
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

#ifndef SENTRYPEER_TEST_DATABASE_H
#define SENTRYPEER_TEST_DATABASE_H 1

#define TEST_DB_FILE "test_sentrypeer.db"

int test_setup_sqlite_db(void **state);
int test_teardown_sqlite_db(void **state);

void test_open_select_close_sqlite_db(void **state);
void test_db_insert_bad_actor(void **state);
void test_db_select_bad_actor(void **state);
void test_db_select_bad_actors(void **state);

#endif //SENTRYPEER_TEST_DATABASE_H
