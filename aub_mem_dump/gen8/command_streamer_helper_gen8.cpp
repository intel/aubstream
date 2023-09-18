/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen8/command_streamer_helper_gen8.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x000020d8, 0x00020000),
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperBcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperVcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperVecs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

GpuGen8::GpuGen8() {
    commandStreamerHelperTable[0][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperGen8<CommandStreamerHelperRcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperGen8<CommandStreamerHelperBcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperGen8<CommandStreamerHelperVcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperGen8<CommandStreamerHelperVecs>>(0);
}

} // namespace aub_stream
