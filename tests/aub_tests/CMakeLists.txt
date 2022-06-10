#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TARGET_NAME ${PROJECT_NAME}_aub_tests)
message(STATUS "AUB tests: ${TARGET_NAME}")

add_executable(${TARGET_NAME}
               ${SOURCES}
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_mem_dump_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_tests_filters.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/simple_batch_buffer_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/simple_batch_buffer_fixture.h
)

get_property(TEST_TRAITS_MACROS GLOBAL PROPERTY TEST_TRAITS_MACROS_property)

target_compile_definitions(${TARGET_NAME} PUBLIC ${TEST_TRAITS_MACROS})

if(WIN32)
  target_link_libraries(${TARGET_NAME} Ws2_32_stub)
endif()

set_target_properties(
                      ${TARGET_NAME}
                      PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
                      VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=* --gtest_catch_exceptions=0"
)

set(RUN_UNIT_TESTS_TARGET run_${TARGET_NAME})
add_custom_target(${RUN_UNIT_TESTS_TARGET} ALL DEPENDS ${TARGET_NAME})
add_custom_command(
                   TARGET ${RUN_UNIT_TESTS_TARGET}
                   POST_BUILD
                   COMMAND echo Running ${TARGET_NAME} in $<TARGET_FILE_DIR:${TARGET_NAME}>
                   COMMAND WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_NAME}>
                   COMMAND ${AUBSTREAM_RUN_INTERCEPTOR_LIST} ${TARGET_NAME} --gtest_catch_exceptions=0
)
if(DO_NOT_RUN_AUB_TESTS)
  set_target_properties(run_${TARGET_NAME} PROPERTIES
                        EXCLUDE_FROM_DEFAULT_BUILD TRUE
                        EXCLUDE_FROM_ALL TRUE
  )
endif()

if(NOT ("${BRANCH_TYPE}" STREQUAL ""))
  add_subdirectory("${BRANCH_TYPE}")
endif()
aub_stream_create_source_tree(${TARGET_NAME})
