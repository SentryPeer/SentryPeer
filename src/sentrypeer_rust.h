/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
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


#ifndef SENTRYPEER_RUST_H
#define SENTRYPEER_RUST_H

/* Generated with cbindgen:0.27.0 */

/* Warning, do not modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "conf.h"

/**
 * A manually created struct to represent a BadActor from bad_actor.h
 */
typedef struct BadActor
{
    const char *event_timestamp;
    const char *event_uuid;
    const char *collected_method;
    const char *created_by_node_id;
    const char *sip_message;
    const char *source_ip;
    const char *destination_ip;
    const char *called_number;
    const char *method;
    const char *transport_type;
    const char *user_agent;
    const char *seen_last;
    const char *seen_count;
} BadActor;

/**
 * Initialize a BadActor struct and return a pointer to it
 */
struct BadActor *return_bad_actor_new(const char *sip_message,
                                      const char *source_ip,
                                      const char *destination_ip,
                                      const char *called_number,
                                      const char *method,
                                      const char *transport_type,
                                      const char *user_agent,
                                      const char *collected_method,
                                      const char *created_by_node_id);

/**
 * # Safety
 *
 * This function is unsafe because it dereferences a raw pointer for
 * the whole BadActor struct and its CString fields (from_raw).
 *
 * Destroy a BadActor struct
 */
void bad_actor_free(struct BadActor *bad_actor);

/**
 * The simplest function used to confirm that calling our Rust library from C is working
 */
void display_rust(void);

/**
 * Return libc::EXIT_SUCCESS or libc::EXIT_FAILURE depending on the function argument
 */
int32_t return_exit_status(bool success);

/**
 * # Safety
 *
 * Return a string
 *
 * The caller is responsible for freeing the string. Generally, the caller
 * from the C FFI side.
 */
char *return_string(void);

/**
 * # Safety
 *
 * Free the string allocated by into_raw from return_string
 */
void free_string(char *ptr_s);

/**
 * # Safety
 *
 * This function takes a function pointer as an argument and calls it, so we can pass them
 * in from C and call it from Rust - a callback inside a thread or loop.
 */
int32_t callback_from_c(int32_t (*callback)(bool), bool success);

/**
 * # Safety
 *
 * Nothing is done with the `sentrypeer_config` pointer, it's treated read-only.
 */
int32_t listen_tls(sentrypeer_config *sentrypeer_c_config);

#endif  /* SENTRYPEER_RUST_H */
