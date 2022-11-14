/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpc_core/pvc/command_streamer_helper_xe_hpc_core_pvc.h"
#include "aub_mem_dump/page_table_pml5.h"

namespace aub_stream {

template <>
const Gpu *enableGpu<ProductFamily::Pvc>() {
    static const GpuPvc pvc;
    return &pvc;
}
} // namespace aub_stream
