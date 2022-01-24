# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
#
#   _____            _              _____
#  / ____|          | |            |  __ \
# | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
#  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
#  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
# |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
#                              __/ |
#                             |___/
#
cmake_minimum_required(VERSION 3.20)
project(sentrypeer
        VERSION 1.0.0
        HOMEPAGE_URL https://github.com/SentryPeer/SentryPeer
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(sentrypeer
        ${CMAKE_SOURCE_DIR}/src/sentrypeer.c
        ${CMAKE_SOURCE_DIR}/src/conf.c
        ${CMAKE_SOURCE_DIR}/src/http_common.c
        ${CMAKE_SOURCE_DIR}/src/http_daemon.c
        ${CMAKE_SOURCE_DIR}/src/http_routes.c
        ${CMAKE_SOURCE_DIR}/src/http_route_regex_check.c
        ${CMAKE_SOURCE_DIR}/src/http_health_check_route.c
        ${CMAKE_SOURCE_DIR}/src/http_ip_addresses_route.c
        ${CMAKE_SOURCE_DIR}/src/http_ip_address_route.c
        ${CMAKE_SOURCE_DIR}/src/http_called_numbers_route.c
        ${CMAKE_SOURCE_DIR}/src/http_called_number_route.c
        ${CMAKE_SOURCE_DIR}/src/sip_daemon.c
        ${CMAKE_SOURCE_DIR}/src/sip_parser.c
        ${CMAKE_SOURCE_DIR}/src/utils.c
        ${CMAKE_SOURCE_DIR}/src/bad_actor.c
        ${CMAKE_SOURCE_DIR}/src/database.c
        )

# See https://github.com/bast/cmake-example/blob/master/CMakeLists.txt
# get git hash
include(${CMAKE_SOURCE_DIR}/cmake/git_revision.cmake)

# project version
set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})

target_link_libraries(sentrypeer -losipparser2)
target_link_libraries(sentrypeer -lsqlite3)
target_link_libraries(sentrypeer -luuid)
target_link_libraries(sentrypeer -lmicrohttpd)
target_link_libraries(sentrypeer -ljansson)
target_link_libraries(sentrypeer -lcurl)

# configure header file
configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/config.h.in
        ${CMAKE_SOURCE_DIR}/src/config.h
)
