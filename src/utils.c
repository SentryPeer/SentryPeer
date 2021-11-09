/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#define _GNU_SOURCE
#include "utils.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>

void print_event_timestamp(void)
{
	struct timespec timestamp_ts;
	char timestamp_buf[TIMESTAMP_LEN];
	if (clock_gettime(CLOCK_REALTIME, &timestamp_ts) == -1) {
		perror("clock_gettime() failed.");
	}

	assert(timestamp_ts.tv_nsec < 1000000000);

	struct tm time_info;
	localtime_r(&timestamp_ts.tv_sec, &time_info);
	strftime(timestamp_buf, TIMESTAMP_LEN, "%Y-%m-%d %H:%M:%S", &time_info);
	fprintf(stderr, "%s.%06ld\n", timestamp_buf, timestamp_ts.tv_nsec);
}
