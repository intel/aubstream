#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(SUPPORT_ADLP)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_gen12lp_adlp.cpp
  )
endif()
