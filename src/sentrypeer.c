/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdio.h>
#include <stdlib.h>

#include "sentrypeer.h"

int main(void) {

  fprintf(stdout, "Hello from %s, version: %s\n", SENTRYPEER_NAME,
          SENTRYPEER_VERSION);

  return EXIT_SUCCESS;
}