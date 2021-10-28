/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "sentrypeer.h"

int main(void) {

  fprintf(stdout, "Hello from '%s'\nversion: %s\nrevision: %s \n", PACKAGE_NAME,
          PACKAGE_VERSION, REVISION);

  return EXIT_SUCCESS;
}