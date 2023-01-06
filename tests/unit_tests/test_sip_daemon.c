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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <stdlib.h>
#include "../../src/sip_daemon.h"

void test_sip_daemon(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	assert_non_null(config);

	assert_int_equal(sip_daemon_run(config), EXIT_SUCCESS);
	assert_int_equal(sip_daemon_stop(config), EXIT_SUCCESS);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}
