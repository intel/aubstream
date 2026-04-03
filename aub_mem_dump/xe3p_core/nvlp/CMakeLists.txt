#
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_SUPPORT_NVLP)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3p_core_nvlp.cpp
  )

  target_sources(${PROJECT_NAME}_all_hw
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/enable_product_nvlp.cpp
  )

endif()
