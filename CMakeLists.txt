#
# Copyright (C) 2022-2023 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(MSVC)
  cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
elseif()
  cmake_minimum_required(VERSION 3.2.0 FATAL_ERROR)
endif()

# Set custom build types for multi-configuration generators
set(CMAKE_CONFIGURATION_TYPES "Release;ReleaseInternal;Debug")

if(NOT BUILD_BITS)
  set(BUILD_BITS 64)
endif()

if(BUILD_BITS EQUAL 64)
  set(ARCH "x64")
else()
  if(NOT WIN32)
    message(FATAL_ERROR "Linux x86 Unsupported!")
  endif()
  set(ARCH "x86")
endif()

if(DEFINED CMAKE_BUILD_TYPE)
  get_property(is_mc GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(${is_mc})
    message(WARNING "Invalid use of CMAKE_BUILD_TYPE with multi-configuration generator")
  endif()
  list(FIND CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} build_found)
  if(NOT "${build_found}" STREQUAL "-1")
    message(STATUS "Build config: ${CMAKE_BUILD_TYPE} ${ARCH}")
  else()
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
  endif()
  unset(build_found)
else()
  message(STATUS "Build configations: ${CMAKE_CONFIGURATION_TYPES} ${ARCH}")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Override project name if requested
if(NOT DEFINED AUB_STREAM_PROJECT_NAME)
  set(AUB_STREAM_PROJECT_NAME aub_stream)
endif()
project(${AUB_STREAM_PROJECT_NAME} LANGUAGES CXX)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.branch" AND NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.branch")
  file(READ ".branch" BRANCH_TYPE)
  string(STRIP "${BRANCH_TYPE}" BRANCH_TYPE)
endif()

MESSAGE(STATUS "Project: ${PROJECT_NAME}")
MESSAGE(STATUS "Branch used: ${BRANCH_TYPE}")
set(BRANCH_DIR_SUFFIX "/${BRANCH_TYPE}")
if(NOT "${BRANCH_TYPE}" STREQUAL "")
  set(BRANCH_DIR_SUFFIX "${BRANCH_DIR_SUFFIX}/")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/init_platforms.cmake)

# set output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Do not add rpath
set(CMAKE_SKIP_RPATH YES)

string(REPLACE " " ";" AUBSTREAM_RUN_INTERCEPTOR_LIST "${AUBSTREAM_RUN_INTERCEPTOR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

  # treat warnings as errors
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W3 /we4189")

  string(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions(-D_HAS_STATIC_RTTI=0)
endif()

if(MSVC_IDE)
  # enable multi-process compilation
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# Setup ReleaseInternal config based on Release
set(CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASEINTERNAL=1 -D_INTERNAL=1")
set(CMAKE_LINK_FLAGS_RELEASEINTERNAL "${CMAKE_LINK_FLAGS_RELEASE}")

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DAUBSTREAM_SETTINGS_DISABLE=1")
endif()

option(AUBSTREAM_CCACHE_ALLOWED "allow use of ccache" TRUE)
find_program(CCACHE_EXE_FOUND ccache)
if(CCACHE_EXE_FOUND AND AUBSTREAM_CCACHE_ALLOWED)
  message(STATUS "Found ccache: ${CCACHE_EXE_FOUND}")
  set_property(DIRECTORY PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(DIRECTORY PROPERTY RULE_LAUNCH_LINK ccache)
endif()

if(NOT DEFINED AUB_STREAM_DIR)
  set(AUB_STREAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

function(aub_stream_create_source_tree target)
  if(MSVC)
    get_target_property(source_list ${target} SOURCES)
    source_group(TREE ${AUB_STREAM_DIR} FILES ${source_list})
  endif()
endfunction()

function(aub_stream_add_subdirectories dirmask)
  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} ${dirmask})
  foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
  endforeach()
endfunction()

set(INTERFACE_HEADERS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(NOT ("${BRANCH_TYPE}" STREQUAL ""))
  set(INTERFACE_HEADERS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/include)
endif()
include_directories(${INTERFACE_HEADERS_DIRECTORY})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(NOT DEFINED THIRD_PARTY_DIR)
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../third_party")
    get_filename_component(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party" ABSOLUTE)
  else()
    set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
  endif()
endif()

set(SOURCES
    ${SOURCES}
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/allocation_params.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/aub_manager.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/aubstream.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/engine_node.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/hardware_context.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/page_info.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/shared_mem_info.h
    ${INTERFACE_HEADERS_DIRECTORY}/aubstream/physical_allocation_info.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

MESSAGE(STATUS "Library name: ${PROJECT_NAME}")

add_subdirectory(aub_mem_dump)

# Only build tests if requested
option(aubstream_build_tests "Build AubStream's unit tests." OFF)
if(aubstream_build_tests)
  # Only add googlemock support if not already added
  if(NOT TARGET gmock)
    if(NOT DEFINED GOOGLE_MOCK_DIR)
      set(GOOGLE_MOCK_DIR ${THIRD_PARTY_DIR}/googletest/googlemock)
    endif()
    message(STATUS "checking ${GOOGLE_MOCK_DIR}...")

    # Ensure googlemock exists
    if(EXISTS "${GOOGLE_MOCK_DIR}/include/gmock")
      message(STATUS "Googlemock: ${GOOGLE_MOCK_DIR}")
      set(GOOGLETEST_VERSION 1.11.0)
      add_subdirectory(${GOOGLE_MOCK_DIR} ${CMAKE_BINARY_DIR}/googlemock EXCLUDE_FROM_ALL)

      add_subdirectory(tests)
    else()
      message(FATAL_ERROR "Googlemock NOT FOUND. Unable to build tests.")
    endif()
  endif()
endif()

