#
# Copyright (C) 2022-2023 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TARGET_NAME ${PROJECT_NAME}_unit_tests)
message(STATUS "Unit tests: ${TARGET_NAME}")

add_executable(${TARGET_NAME}
               ${SOURCES}
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_manager_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_stream_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/aub_tbx_stream_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests.h
               ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/stolen_memory_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/test_align_helpers.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/test_misc_helpers.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_stolen_memory.h
               ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/legacy_page_table_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/memory_bank_helper_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_aub_manager.h
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_aub_stream.h
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_gpu.h
               ${CMAKE_CURRENT_SOURCE_DIR}/mock_physical_address_allocator.h
               ${CMAKE_CURRENT_SOURCE_DIR}/options_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/page_table_helper.h
               ${CMAKE_CURRENT_SOURCE_DIR}/page_table_pml5_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/page_table_walker_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/physical_allocator_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/tbx_socket_tests.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/white_box.h
               ${CMAKE_SOURCE_DIR}/tests/empty_test_filters.cpp
)

get_property(TEST_TRAITS_MACROS GLOBAL PROPERTY TEST_TRAITS_MACROS_property)

target_compile_definitions(${TARGET_NAME} PUBLIC ${TEST_TRAITS_MACROS})

if(WIN32)
  target_link_libraries(${TARGET_NAME} Ws2_32_stub)
else()
  target_link_libraries(${TARGET_NAME} socket_stubs)
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

if(NOT ("${BRANCH_TYPE}" STREQUAL ""))
  add_subdirectory("${BRANCH_TYPE}")
endif()

aub_stream_add_subdirectories("gen*")
aub_stream_add_subdirectories("*_core")
aub_stream_create_source_tree(${TARGET_NAME})
