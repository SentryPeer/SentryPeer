/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_TEST_DATABASE_H
#define SENTRYPEER_TEST_DATABASE_H 1

void test_open_add_close_sqlite_db(void **state);
void test_db_insert_bad_actor(void **state);

#endif //SENTRYPEER_TEST_DATABASE_H
