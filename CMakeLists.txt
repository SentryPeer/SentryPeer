cmake_minimum_required(VERSION 3.20)
project(sentrypeer
        VERSION 0.0.1
        HOMEPAGE_URL https://github.com/SentryPeer/SentryPeer
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(sentrypeer
        ${CMAKE_SOURCE_DIR}/src/sentrypeer.c
        ${CMAKE_SOURCE_DIR}/src/conf.c
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

# configure header file
configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/config.h.in
        ${CMAKE_SOURCE_DIR}/src/config.h
)
