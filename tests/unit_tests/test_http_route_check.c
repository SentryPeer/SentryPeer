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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>

#include "test_http_route_check.h"
#include "../../src/http_routes.h"

void test_http_route_check(void **state)
{
	(void)state; /* unused */

	sentrypeer_config *config = sentrypeer_config_new();
	config->verbose_mode = true;

	char requested_url[] = "/test/route/check";

	char route0[] = "/";
	char route1[] = "/ip-addresses/8.8.8.8";
	char route2[] = "/ip-addresses";
	char route3[] = "/health-check/";
	char route4[] = "/health-check";
	char route5[] = "/numbers";
	char route6[] = "/numbers/123456789";

	// Expect no matches.
	assert_int_not_equal(route_check(route0, requested_url, config), 0);
	assert_int_not_equal(route_check(route1, requested_url, config), 0);
	assert_int_not_equal(route_check(route2, requested_url, config), 0);
	assert_int_not_equal(route_check(route3, requested_url, config), 0);
	assert_int_not_equal(route_check(route4, requested_url, config), 0);
	assert_int_not_equal(route_check(route5, requested_url, config), 0);
	assert_int_not_equal(route_check(route6, requested_url, config), 0);

	// Test a route that is smaller than all, but more than "/". Expect no matches.
	char requested_url_middle_size[] = "/test/rt";

	assert_int_not_equal(
		route_check(route0, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route1, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route2, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route3, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route4, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route5, requested_url_middle_size, config), 0);
	assert_int_not_equal(
		route_check(route6, requested_url_middle_size, config), 0);

	// Test a route that is smaller than all, but has a partial match, apart from end of url. Expect no matches.
	char requested_url_middle_size_partial[] = "/ip-/";

	assert_int_not_equal(
		route_check(route0, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route1, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route2, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route3, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route4, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route5, requested_url_middle_size_partial, config),
		0);
	assert_int_not_equal(
		route_check(route6, requested_url_middle_size_partial, config),
		0);

	// Test a route that is smaller than all, but has a partial match. No end of url. Expect no matches.
	char requested_url_middle_size_partial_no_end[] = "/ip-";

	assert_int_not_equal(
		route_check(route0, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route1, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route2, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route3, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route4, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route5, requested_url_middle_size_partial_no_end,
			    config),
		0);
	assert_int_not_equal(
		route_check(route6, requested_url_middle_size_partial_no_end,
			    config),
		0);

	// Test a match.
	char requested_url_match[] = "/ip-addresses";
	assert_int_equal(route_check(route2, requested_url_match, config), 0);

	// Test a bigger url that than we have a route for. Expect no matches.
	char requested_url_bigger[] =
		"/ip-this-is-longer-url-than-we-have-a-route-for-but-has-ip-prefix/";
	assert_int_not_equal(route_check(route2, requested_url_bigger, config),
			     0);

	sentrypeer_config_destroy(&config);
	assert_null(config);
}
