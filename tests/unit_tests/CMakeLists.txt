if (UNIT_TESTING)
    set(TEST_RUNNER_NAME runner)

    # Find opendht_c
    find_library(OPENDHT_C opendht-c)

    # configure header file
    configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/config.h.in
            ${CMAKE_SOURCE_DIR}/tests/unit_tests/config.h
    )

    add_executable(${TEST_RUNNER_NAME}
    ${CMAKE_SOURCE_DIR}/src/sip_message_event.c
    ${CMAKE_SOURCE_DIR}/src/sip_daemon.c
    ${CMAKE_SOURCE_DIR}/src/http_common.c
    ${CMAKE_SOURCE_DIR}/src/http_daemon.c
    ${CMAKE_SOURCE_DIR}/src/http_routes.c
    ${CMAKE_SOURCE_DIR}/src/http_health_check_route.c
    ${CMAKE_SOURCE_DIR}/src/http_ip_addresses_route.c
    ${CMAKE_SOURCE_DIR}/src/http_ip_address_route.c
    ${CMAKE_SOURCE_DIR}/src/http_called_numbers_route.c
    ${CMAKE_SOURCE_DIR}/src/http_called_number_route.c
    ${CMAKE_SOURCE_DIR}/src/regex_match.c
    ${CMAKE_SOURCE_DIR}/src/sip_parser.c
    ${CMAKE_SOURCE_DIR}/src/bad_actor.c
    ${CMAKE_SOURCE_DIR}/src/conf.c
    ${CMAKE_SOURCE_DIR}/src/json_logger.c
    ${CMAKE_SOURCE_DIR}/src/utils.c
    ${CMAKE_SOURCE_DIR}/src/database.c
    ${CMAKE_SOURCE_DIR}/src/peer_to_peer_dht.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_conf.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_json_logger.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_utils.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_bad_actor.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_database.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_http_api.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_http_api_version.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_http_route_check.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_ip_address_regex.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_sip_message_event.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_sip_daemon.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_sentrypeer_rust.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_peer_to_peer_dht.c
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/runner.c
    )

    target_include_directories(${TEST_RUNNER_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
    target_include_directories(${TEST_RUNNER_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/tests/unit_tests)

    target_link_libraries(${TEST_RUNNER_NAME} -losipparser2)
    target_link_libraries(${TEST_RUNNER_NAME} -lsqlite3)
    target_link_libraries(${TEST_RUNNER_NAME} -luuid)
    target_link_libraries(${TEST_RUNNER_NAME} -lmicrohttpd)
    target_link_libraries(${TEST_RUNNER_NAME} -ljansson)
    target_link_libraries(${TEST_RUNNER_NAME} -lcurl)
    target_link_libraries(${TEST_RUNNER_NAME} -lpcre2-8)
    target_link_libraries(${TEST_RUNNER_NAME} -lopendht-c)
    target_link_libraries(${TEST_RUNNER_NAME} sentrypeer_rust)
    target_link_libraries(${TEST_RUNNER_NAME} -lcmocka)

    enable_testing()
    add_test(NAME ${TEST_RUNNER_NAME} COMMAND ${TEST_RUNNER_NAME})
    add_test(
        NAME sentrypeer_rust
        COMMAND cargo test --manifest-path ${CMAKE_SOURCE_DIR}/sentrypeer_rust/Cargo.toml -- --nocapture
    )

endif()