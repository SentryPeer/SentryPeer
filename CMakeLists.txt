# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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
cmake_minimum_required(VERSION 3.22)
project(sentrypeer
        VERSION 4.0.5
        HOMEPAGE_URL https://github.com/SentryPeer/SentryPeer
        LANGUAGES C)

set(CMAKE_C_STANDARD 23)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(FindPkgConfig)

option(UNIT_TESTING "Enable unit testing" OFF)
option(DISABLE_OPENDHT "Disable OpenDHT support" OFF)
option(DISABLE_RUST "Disable Rust parts" OFF)
option(RUST_DEBUG_RELEASE "Rust debug or release" OFF)

if (DISABLE_OPENDHT)
    add_definitions(-DHAVE_OPENDHT=0)
    message(STATUS "OpenDHT support disabled")
endif ()

if (DISABLE_RUST)
    add_definitions(-DHAVE_RUST=0)
    message(STATUS "Rust support disabled")
endif ()

if (RUST_DEBUG_RELEASE)
    add_definitions(-DRUST_DEBUG_RELEASE=1)
    message(STATUS "Rust debug release enabled")
endif ()

set(SRC_FILES ${CMAKE_SOURCE_DIR}/src/sentrypeer.c
        ${CMAKE_SOURCE_DIR}/src/signal_handler.c
        ${CMAKE_SOURCE_DIR}/src/conf.c
        ${CMAKE_SOURCE_DIR}/src/http_common.c
        ${CMAKE_SOURCE_DIR}/src/http_daemon.c
        ${CMAKE_SOURCE_DIR}/src/http_routes.c
        ${CMAKE_SOURCE_DIR}/src/http_health_check_route.c
        ${CMAKE_SOURCE_DIR}/src/http_ip_addresses_route.c
        ${CMAKE_SOURCE_DIR}/src/http_ip_address_route.c
        ${CMAKE_SOURCE_DIR}/src/http_called_numbers_route.c
        ${CMAKE_SOURCE_DIR}/src/http_called_number_route.c
        ${CMAKE_SOURCE_DIR}/src/regex_match.c
        ${CMAKE_SOURCE_DIR}/src/sip_message_event.c
        ${CMAKE_SOURCE_DIR}/src/sip_daemon.c
        ${CMAKE_SOURCE_DIR}/src/sip_parser.c
        ${CMAKE_SOURCE_DIR}/src/peer_to_peer_dht.c
        ${CMAKE_SOURCE_DIR}/src/utils.c
        ${CMAKE_SOURCE_DIR}/src/bad_actor.c
        ${CMAKE_SOURCE_DIR}/src/database.c
        ${CMAKE_SOURCE_DIR}/src/json_logger.c
)

# Build a SentryPeer library for use in our Rust code FIRST as,
# our Rust code depends on it in our executable
add_library(${CMAKE_PROJECT_NAME}_lib SHARED
        ${SRC_FILES}
)

# Make the library have the same name as the project
# https://discourse.cmake.org/t/how-to-use-the-same-name-for-a-target-and-a-library/5044/2
set_target_properties(${CMAKE_PROJECT_NAME}_lib PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})

add_executable(${CMAKE_PROJECT_NAME}
        ${SRC_FILES}
)

# See https://github.com/bast/cmake-example/blob/master/CMakeLists.txt
# get git hash
include(${CMAKE_SOURCE_DIR}/cmake/git_revision.cmake)

find_package(PkgConfig REQUIRED)

pkg_search_module(OSIP2 REQUIRED libosip2)
pkg_search_module(SQLITE3 REQUIRED sqlite3)
pkg_search_module(UUID REQUIRED uuid)
pkg_search_module(MICROHTTPD REQUIRED libmicrohttpd)
pkg_search_module(JANSSON REQUIRED jansson)
pkg_search_module(CURL REQUIRED libcurl)
pkg_search_module(PCRE2 REQUIRED libpcre2-8)

if (NOT DISABLE_OPENDHT)
    pkg_search_module(OPENDHT opendht)
    message(STATUS "OpenDHT support not disabled")
endif ()

if (OPENDHT_FOUND)
    add_definitions(-DHAVE_OPENDHT_C=1)
    message(STATUS "OPENDHT_C_VERSION: ${OPENDHT_VERSION}")
    add_definitions(-DOPENDHT_C_VERSION="${OPENDHT_VERISON}")
endif ()

include_directories(${OSIP2_INCLUDE_DIRS})
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${UUID_INCLUDE_DIRS})
include_directories(${MICROHTTPD_INCLUDE_DIRS})
include_directories(${JANSSON_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${PCRE2_INCLUDE_DIRS})
include_directories(${OPENDHT_INCLUDE_DIRS})

# project version
set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})

# For our Rust code
# See https://github.com/corrosion-rs/corrosion
if (NOT DISABLE_RUST)
    include(FetchContent)

    FetchContent_Declare(
            Corrosion
            GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    )
    FetchContent_MakeAvailable(Corrosion)

    # Import targets defined in a package or workspace manifest `Cargo.toml` file
    corrosion_import_crate(MANIFEST_PATH sentrypeer_rust/Cargo.toml)
    message(STATUS "Rust support enabled")

    # For config.h.in
    set(HAVE_RUST 1)
    add_definitions(-DHAVE_RUST=1)
endif ()

target_link_libraries(${CMAKE_PROJECT_NAME} -losipparser2)
target_link_libraries(${CMAKE_PROJECT_NAME} -lsqlite3)
target_link_libraries(${CMAKE_PROJECT_NAME} -luuid)
target_link_libraries(${CMAKE_PROJECT_NAME} -lmicrohttpd)
target_link_libraries(${CMAKE_PROJECT_NAME} -ljansson)
target_link_libraries(${CMAKE_PROJECT_NAME} -lcurl)
target_link_libraries(${CMAKE_PROJECT_NAME} -lpcre2-8)

# Used in config.h.in - can't reset OPENDHT_FOUND here, so use a new variable
if (NOT OPENDHT_FOUND)
    set(HAVE_OPENDHT_C 0)
endif ()

if (OPENDHT_FOUND AND NOT DISABLE_OPENDHT)
    target_link_libraries(${CMAKE_PROJECT_NAME} -lopendht-c)
    message(STATUS "Linking with OpenDHT")
    set(HAVE_OPENDHT_C 1)
endif ()

if (NOT DISABLE_RUST)
    target_link_libraries(${CMAKE_PROJECT_NAME} sentrypeer_rust)
    message(STATUS "Linking with Rust parts")
endif ()

# configure header files
configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/config.h.in
        ${CMAKE_SOURCE_DIR}/config.h
)

configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/config.h.in
        ${CMAKE_SOURCE_DIR}/src/config.h
)

if (UNIT_TESTING)
    include(CTest)
    find_package(cmocka 1.1.5 REQUIRED)
    add_subdirectory(tests/unit_tests)
    message(STATUS "Unit testing enabled")
endif ()

