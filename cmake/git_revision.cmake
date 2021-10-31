# See https://github.com/bast/cmake-example/blob/master/cmake/git_revision.cmake

find_package(Git)

if(GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            OUTPUT_VARIABLE REVISION
            ERROR_QUIET
    )
    if(NOT ${REVISION} STREQUAL "")
        string(STRIP ${REVISION} REVISION)
    endif()
    message(STATUS "Current git revision is ${REVISION}")
else()
    set(REVISION "not found.")
endif()
