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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_ip_address_regex.h"

#define PCRE2_CODE_UNIT_WIDTH 8

#include <stdio.h>
#include <string.h>
#include <pcre2.h>

void test_ip_address_regex(void **state)
{
	(void)state; /* unused */

	pcre2_code *re;
	PCRE2_SPTR
	pattern; /* PCRE2_SPTR is a pointer to unsigned code units of */
	PCRE2_SPTR subject; /* the appropriate width (in this case, 8 bits). */

	int errornumber;
	int i;
	int rc;

	PCRE2_SIZE erroroffset;
	PCRE2_SIZE *ovector;
	PCRE2_SIZE subject_length;

	pcre2_match_data *match_data;

	/* Pattern and subject are char arguments, so they can be straightforwardly
cast to PCRE2_SPTR because we are working in 8-bit code units. The subject
length is cast to PCRE2_SIZE for completeness, though PCRE2_SIZE is in fact
defined to be size_t. */

	pattern = (PCRE2_SPTR) "/ip-addresses/(.*)";
	subject = (PCRE2_SPTR) "/ip-addresses/8.8.8.8"; // url from MHD
	subject_length = (PCRE2_SIZE)strlen((char *)subject);

	/*************************************************************************
* Now we are going to compile the regular expression pattern, and handle *
* any errors that are detected.                                          *
*************************************************************************/

	re = pcre2_compile(
		pattern, /* the pattern */
		PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
		0, /* default options */
		&errornumber, /* for error number */
		&erroroffset, /* for error offset */
		NULL); /* use default compile context */

	/* Compilation failed: print the error message and exit. */

	if (re == NULL) {
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
		printf("PCRE2 compilation failed at offset %d: %s\n",
		       (int)erroroffset, buffer);
	}

	/*************************************************************************
* If the compilation succeeded, we call PCRE2 again, in order to do a    *
* pattern match against the subject string. This does just ONE match. If *
* further matching is needed, it will be done below. Before running the  *
* match we must set up a match_data block for holding the result. Using  *
* pcre2_match_data_create_from_pattern() ensures that the block is       *
* exactly the right size for the number of capturing parentheses in the  *
* pattern. If you need to know the actual size of a match_data block as  *
* a number of bytes, you can find it like this:                          *
*                                                                        *
* PCRE2_SIZE match_data_size = pcre2_get_match_data_size(match_data);    *
*************************************************************************/

	match_data = pcre2_match_data_create_from_pattern(re, NULL);

	/* Now run the match. */

	rc = pcre2_match(re, /* the compiled pattern */
			 subject, /* the subject string */
			 subject_length, /* the length of the subject */
			 0, /* start at offset 0 in the subject */
			 0, /* default options */
			 match_data, /* block for storing the result */
			 NULL); /* use default match context */

	/* Matching failed: handle error cases */

	if (rc < 0) {
		switch (rc) {
		case PCRE2_ERROR_NOMATCH:
			printf("No match\n");
			break;
		/* Handle other special cases if you like  */
		default:
			printf("Matching error %d\n", rc);
			break;
		}
		pcre2_match_data_free(
			match_data); /* Release memory used for the match */
		pcre2_code_free(re); /*   data and the compiled pattern. */
	}

	/* Match succeeded. Get a pointer to the output vector, where string offsets
are stored. */

	ovector = pcre2_get_ovector_pointer(match_data);
	printf("Match succeeded at offset %d\n", (int)ovector[0]);

	/*************************************************************************
* We have found the first match within the subject string. If the output *
* vector wasn't big enough, say so. Then output any substrings that were *
* captured.                                                              *
*************************************************************************/

	/* The output vector wasn't big enough. This should not happen, because we used
pcre2_match_data_create_from_pattern() above. */

	if (rc == 0)
		printf("ovector was not big enough for all the captured substrings\n");

	/* Since release 10.38 PCRE2 has locked out the use of \K in lookaround
assertions. However, there is an option to re-enable the old behaviour. If that
is set, it is possible to run patterns such as /(?=.\K)/ that use \K in an
assertion to set the start of a match later than its end. In this demonstration
program, we show how to detect this case, but it shouldn't arise because the
option is never set. */

	if (ovector[0] > ovector[1]) {
		printf("\\K was used in an assertion to set the match start after its end.\n"
		       "From end to start the match was: %.*s\n",
		       (int)(ovector[0] - ovector[1]),
		       (char *)(subject + ovector[1]));
		printf("Run abandoned\n");
		pcre2_match_data_free(match_data);
		pcre2_code_free(re);
	}

	/* Show substrings stored in the output vector by number. Obviously, in a real
application you might want to do things other than print them. */

	for (i = 0; i < rc; i++) {
		PCRE2_SPTR substring_start = subject + ovector[2 * i];
		PCRE2_SIZE substring_length =
			ovector[2 * i + 1] - ovector[2 * i];
		printf("%2d: %.*s\n", i, (int)substring_length,
		       (char *)substring_start);
	}
	// Our capture is the first, so when i = 1, 2 * 1 = 2, hence ovector[2 * 1]
	assert_string_equal("8.8.8.8", (char *)(subject + ovector[2]));

	// Final clean up
	pcre2_match_data_free(
		match_data); /* Release memory used for the match */
	pcre2_code_free(re); /*   data and the compiled pattern. */

	/**************************************************************************
* That concludes the basic part of this demonstration program. We have    *
* compiled a pattern, and performed a single match. The code that follows *
* shows first how to access named substrings, and then how to code for    *
* repeated matches on the same subject.                                   *
**************************************************************************/
}
