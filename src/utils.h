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

#ifndef SENTRYPEER_UTILS_H
#define SENTRYPEER_UTILS_H 1

#define TIMESTAMP_LEN 40
#define UTILS_UUID_STRING_LEN 37

#include <stddef.h>
#include <sys/socket.h>

/**
 * Get the current time suitable for event logging.
 *
 * @param event_timestamp The timestamp to fill.
 * @return The current time in format YYYY-MM-DD HH:MM:SS.XXXXXXXXX
 */
char *event_timestamp(char *event_timestamp);

/**
 * Duplicate a string (must be freed by caller).
 *
 * @param string The string to duplicate.
 * @return A new string with the same contents as the original
 */
char *util_duplicate_string(const char *string);

/**
 * Copy a string (must be freed by caller).
 *
 * @param dest The string to copy to.
 * @param src The string to copy from.
 * @param dest_len The maximum length of the string to copy.
 * @return A pointer to the destination string.
 */
char *util_copy_string(char *dest, const char *src, size_t dest_len);

/**
 * Generate a uuid.
 *
 * @param uuid_string The string to fill.
 * @return A uuid in string format.
 */
char *util_uuid_generate_string(char *uuid_string);

/**
 * Validate an IP address
 *
 * @param ip_address_to_check The "IP address" to check.
 * @return EXIT_SUCCESS (0) if the IP address is valid, EXIT_FAILURE (0) otherwise.
 */
int valid_ip_address_format(const char *ip_address_to_check);

/**
 * Print an IP address
 *
 * @param addr The sockaddr structure to print.
 * @return The IP address in string format.
 */
char *util_addr_string(const struct sockaddr *addr);

#endif //SENTRYPEER_UTILS_H
