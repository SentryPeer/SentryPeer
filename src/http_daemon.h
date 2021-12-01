/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_HTTP_DAEMON_H
#define SENTRYPEER_HTTP_DAEMON_H 1

#include "conf.h"

#define HTTP_DAEMON_PORT 8082

int http_daemon_init(struct sentrypeer_config const *config);

#endif //SENTRYPEER_HTTP_DAEMON_H
