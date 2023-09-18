/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen11/command_streamer_helper_gen11.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperGen11<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x000020d8, 0x00020000),
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen11<CommandStreamerHelperBcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen11<CommandStreamerHelperVcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen11<CommandStreamerHelperVecs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

GpuGen11::GpuGen11() {
    commandStreamerHelperTable[0][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperGen11<CommandStreamerHelperRcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperGen11<CommandStreamerHelperBcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperGen11<CommandStreamerHelperVcs>>(0);
    commandStreamerHelperTable[0][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperGen11<CommandStreamerHelperVecs>>(0);
}
} // namespace aub_stream
