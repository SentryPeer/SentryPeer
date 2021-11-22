/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_UTILS_H
#define SENTRYPEER_UTILS_H 1

#define TIMESTAMP_LEN 30

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

#endif //SENTRYPEER_UTILS_H
