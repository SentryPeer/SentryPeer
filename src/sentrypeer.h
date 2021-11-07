/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_H
#define SENTRYPEER_H 1

// Modern C - Manning. Chapter 6, Section 6.4:
// Takeaway 6.29
// (Forward-declare a struct within a typedef using the same identifier
// as the tag name.
//

typedef struct bad_actor bad_actor;
struct bad_actor {
	char event_timestamp[26];
	char source_ip[100];
	char called_number[100];
	char method[10];
	char uri[150];
	char via[250];
	char contact[250];
	char user_agent[250];
	char country[200];
};

#endif // SENTRYPEER_H
