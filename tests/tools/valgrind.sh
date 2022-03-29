#!/bin/bash
valgrind -s --show-leak-kinds=all --error-exitcode=1 --leak-check=full --leak-resolution=med --track-origins=yes --vgdb=no tests/unit_tests/runner
