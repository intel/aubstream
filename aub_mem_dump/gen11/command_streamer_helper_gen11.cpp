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

static CommandStreamerHelperGen11<CommandStreamerHelperRcs> rcs(0);
static CommandStreamerHelperGen11<CommandStreamerHelperBcs> bcs(0);
static CommandStreamerHelperGen11<CommandStreamerHelperVcs> vcs(0);
static CommandStreamerHelperGen11<CommandStreamerHelperVecs> vecs(0);

static CommandStreamerHelper *commandStreamerHelperTable[EngineType::NUM_ENGINES] = {};

struct PopulateGen11 {
    PopulateGen11() {
        commandStreamerHelperTable[EngineType::ENGINE_RCS] = &rcs;
        commandStreamerHelperTable[EngineType::ENGINE_BCS] = &bcs;
        commandStreamerHelperTable[EngineType::ENGINE_VCS] = &vcs;
        commandStreamerHelperTable[EngineType::ENGINE_VECS] = &vecs;
    }
} populateGen11;

CommandStreamerHelper &GpuGen11::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device == 0);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}

} // namespace aub_stream
