#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TARGET_NAME ${PROJECT_NAME}_tbx_tests)
message(STATUS "TBX tests: ${TARGET_NAME}")

add_executable(${TARGET_NAME}
               ${SOURCES}
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_tbx_stream_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/tbx_stream_tests.cpp
               ${CMAKE_SOURCE_DIR}/tests/empty_test_filters.cpp
)

get_property(TEST_TRAITS_MACROS GLOBAL PROPERTY TEST_TRAITS_MACROS_property)

target_compile_definitions(${TARGET_NAME} PUBLIC ${TEST_TRAITS_MACROS})

if(WIN32)
  target_link_libraries(${TARGET_NAME} Ws2_32)
endif()

set_target_properties(
                      ${TARGET_NAME}
                      PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
                      VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=* --gtest_catch_exceptions=0"
)

aub_stream_create_source_tree(${TARGET_NAME})
