/*
     This file is part of GNUnet.
     Copyright (C) 2020 GNUnet e.V.

     GNUnet is free software: you can redistribute it and/or modify it
     under the terms of the GNU Affero General Public License as published
     by the Free Software Foundation, either version 3 of the License,
     or (at your option) any later version.

     GNUnet is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     SPDX-License-Identifier: AGPL3.0-or-later
 */

/**
 * @author Christian Grothoff
 *
 * @file
 * MHD compatibility definitions for warning-less compile of
 * our code against MHD before and after #MHD_VERSION 0x00097002.
 */
#include <microhttpd.h>

#if MHD_VERSION >= 0x00097002
/**
 * Data type to use for functions return an "MHD result".
 */
#define MHD_RESULT enum MHD_Result

#else

/**
 * Data type to use for functions return an "MHD result".
 */
#define MHD_RESULT int

#endif
