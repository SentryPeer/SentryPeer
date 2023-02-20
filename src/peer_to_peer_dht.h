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

#include "../config.h"

#if HAVE_OPENDHT_C != 0

#ifndef SENTRYPEER_PEER_TO_PEER_DHT_H
#define SENTRYPEER_PEER_TO_PEER_DHT_H 1

#include "conf.h"
#include "bad_actor.h"

int peer_to_peer_dht_run(sentrypeer_config *config);
int peer_to_peer_dht_stop(sentrypeer_config *config);
int peer_to_peer_dht_save(sentrypeer_config *config,
		       bad_actor const *bad_actor_event);

#endif //SENTRYPEER_PEER_TO_PEER_DHT_H

#endif // HAVE_OPENDHT_C

