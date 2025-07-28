#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_core.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_core.h
)
