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

#ifndef DISABLE_OPENDHT  // This is a compile-time flag

#ifndef SENTRYPEER_TEST_PEER_TO_PEER_DHT_H
#define SENTRYPEER_TEST_PEER_TO_PEER_DHT_H 1

void test_peer_to_peer_dht(void **state);

#endif //SENTRYPEER_TEST_PEER_TO_PEER_DHT_H

#endif // DISABLE_OPENDHT
