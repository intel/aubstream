/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen9/command_streamer_helper_gen9.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperGen9<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x000020d8, 0x00020000),
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen9<CommandStreamerHelperBcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen9<CommandStreamerHelperVcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen9<CommandStreamerHelperVecs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

GpuGen9::GpuGen9() {
    commandStreamerHelperTable[0][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperGen9<CommandStreamerHelperRcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperGen9<CommandStreamerHelperBcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperGen9<CommandStreamerHelperVcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperGen9<CommandStreamerHelperVecs>>(0);
}
} // namespace aub_stream
