/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

#define HTTP_ROUTES_MAX_LEN 1024
#define HOME_PAGE_ROUTE "/"
#define HEALTH_CHECK_ROUTE "/health-check"
#define IP_ADDRESSES_ROUTE "/ip-addresses"
#define IP_ADDRESS_ROUTE "/ip-addresses/:ip_address"
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

int health_check_route(struct MHD_Connection *connection);
int ip_addresses_route(struct MHD_Connection *connection,
		       sentrypeer_config *config);

#endif //SENTRYPEER_HTTP_ROUTES_H
