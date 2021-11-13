/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_DATABASE_H
#define SENTRYPEER_DATABASE_H 1

#include <sqlite3.h>

int db_connect(char *db_name);
int db_create(char *db_name);
int db_execute(char *sql_query);
int db_disconnect(char *db_name);

#endif //SENTRYPEER_DATABASE_H
