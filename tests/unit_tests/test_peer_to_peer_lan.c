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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <zyre.h>
#include <config.h>

#include "test_peer_to_peer_lan.h"

#define ROOM "LAN-GROUP"

void test_peer_to_peer_lan(void **state)
{
	(void)state; /* unused */

	const char *name = PACKAGE_NAME;
	zyre_t *node = zyre_new(name);
	assert_non_null(node);

	assert_int_equal(zyre_start(node), EXIT_SUCCESS);
	assert_int_equal(zyre_join(node, ROOM), EXIT_SUCCESS);
	assert_int_equal(zyre_shouts(node, ROOM, "Hello from %s", name), EXIT_SUCCESS);

	zyre_stop(node);
	zyre_destroy(&node);
}
