#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

#Disable compiler warnings related to googletest and TR1 deprecation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")

if(WIN32)
  add_library(Ws2_32_stub SHARED ${CMAKE_CURRENT_SOURCE_DIR}/socket_stub.cpp)
else()
  add_library(socket_stubs STATIC ${CMAKE_CURRENT_SOURCE_DIR}/socket_stub.cpp)
endif()

add_subdirectory(test_traits)

get_property(TEST_TRAITS_SOURCES GLOBAL PROPERTY TEST_TRAITS_SOURCES_property)

set(SOURCES
    ${SOURCES}
    ${TEST_TRAITS_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_batch_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_batch_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test_defaults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_defaults.h
    ${CMAKE_CURRENT_SOURCE_DIR}/variable_backup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock_os_calls.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(BEFORE $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)

link_libraries(
               ${PROJECT_NAME}
               ${PROJECT_NAME}_all_hw
               gmock
)

add_subdirectory(aub_tests)
add_subdirectory(tbx_tests)
add_subdirectory(unit_tests)
