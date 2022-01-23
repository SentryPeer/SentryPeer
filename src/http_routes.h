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

#ifndef SENTRYPEER_HTTP_ROUTES_H
#define SENTRYPEER_HTTP_ROUTES_H 1

#define HOME_PAGE_ROUTE "/"
#define HEALTH_CHECK_ROUTE "/health-check"
#define IP_ADDRESSES_ROUTE "/ip-addresses"
// ./tests/tools/pcre2demo "/ip-addresses/(.+)" "/ip-addresses/8.8.8.8"
#define IP_ADDRESS_ROUTE "/ip-addresses/(.+)"
#define IP_ADDRESSES_IPSET_ROUTE "/ip-addresses/ipset"
#define NUMBERS_ROUTE "/numbers"
#define NUMBER_ROUTE "/numbers/:number"
#define COUNTRIES_ROUTE "/countries"
#define COUNTRY_ROUTE "/countries/:country"
#define COUNTRY_CITY_ROUTE "/countries/:country/:city"
#define USER_AGENTS_ROUTE "/user-agents"
#define USER_AGENT_ROUTE "/user-agents/:user_agent"
#define SIP_METHODS_ROUTE "/sip-methods"
#define SIP_METHOD_ROUTE "/sip-methods/:sip_method"

#include <microhttpd.h>
#include "conf.h"

enum MHD_Result route_handler(void *cls, struct MHD_Connection *connection,
			      const char *url, const char *method,
			      const char *version, const char *upload_data,
			      size_t *upload_data_size, void **ptr);

int route_check(const char *url, const char *route,
		sentrypeer_config const *config);
int route_regex_check(const char *url, const char *regex, char **matched_string,
		      sentrypeer_config const *config);
int health_check_route(struct MHD_Connection *connection);
int ip_addresses_route(struct MHD_Connection *connection,
		       sentrypeer_config const *config);
int ip_address_route(char **ip_address, struct MHD_Connection *connection,
		     sentrypeer_config const *config);
int called_numbers_route(struct MHD_Connection *connection,
			 sentrypeer_config const *config);

#endif //SENTRYPEER_HTTP_ROUTES_H
