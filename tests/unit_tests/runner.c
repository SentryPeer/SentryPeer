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

// https://git.cryptomilk.org/projects/cmocka.git/tree/example/simple_test.c

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <config.h>

#include "test_conf.h"
#include "test_json_logger.h"
#include "test_utils.h"
#include "test_bad_actor.h"
#include "test_database.h"
#include "test_http_api.h"
#include "test_http_route_check.h"
#include "test_ip_address_regex.h"
#include "test_sip_message.h"
#include "test_sip_daemon.h"

#if HAVE_ZYRE != 0
#include "test_peer_to_peer_lan.h"
#endif

#if HAVE_OPENDHT_C != 0
#include "test_peer_to_peer_dht.h"
#endif

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state)
{
	(void)state; /* unused */
}

int main(void)
{
	// Use XML output here as it shows the time taken for each test. No
	// other output types show this information.
	cmocka_set_message_output(CM_OUTPUT_XML);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(null_test_success),
		cmocka_unit_test(test_conf),
		cmocka_unit_test(test_utils),
		cmocka_unit_test(test_bad_actor),
		cmocka_unit_test(test_bad_actors),
		cmocka_unit_test_setup_teardown(
			test_open_select_close_sqlite_db, test_setup_sqlite_db,
			test_teardown_sqlite_db),
		cmocka_unit_test_setup_teardown(test_db_insert_bad_actor,
						test_setup_sqlite_db,
						test_teardown_sqlite_db),
		cmocka_unit_test_setup_teardown(test_db_select_bad_actor,
						test_setup_sqlite_db,
						test_teardown_sqlite_db),
		cmocka_unit_test_setup_teardown(test_db_select_bad_actors,
						test_setup_sqlite_db,
						test_teardown_sqlite_db),
		cmocka_unit_test_setup_teardown(test_http_api_get,
						test_setup_sqlite_db,
						test_teardown_sqlite_db),
		cmocka_unit_test(test_http_route_check),
		cmocka_unit_test(test_ip_address_regex),
		cmocka_unit_test(test_route_regex_check),
		cmocka_unit_test(test_sip_message),
		cmocka_unit_test(test_sip_daemon),
		cmocka_unit_test_setup_teardown(test_json_logger,
						test_setup_sqlite_db,
						test_teardown_sqlite_db),

#if HAVE_ZYRE != 0
		cmocka_unit_test(test_peer_to_peer_lan),
#endif

#if HAVE_OPENDHT_C != 0
		cmocka_unit_test(test_peer_to_peer_dht),
#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
