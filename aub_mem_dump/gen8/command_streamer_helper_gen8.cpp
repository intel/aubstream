/*
 * Copyright (C) 2022 Intel Corporation
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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperVcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen8<CommandStreamerHelperVecs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

static CommandStreamerHelperGen8<CommandStreamerHelperRcs> rcs(0);
static CommandStreamerHelperGen8<CommandStreamerHelperBcs> bcs(0);
static CommandStreamerHelperGen8<CommandStreamerHelperVcs> vcs(0);
static CommandStreamerHelperGen8<CommandStreamerHelperVecs> vecs(0);

static CommandStreamerHelper *commandStreamerHelperTable[EngineType::NUM_ENGINES] = {};

struct PopulateGen8 {
    PopulateGen8() {
        commandStreamerHelperTable[EngineType::ENGINE_RCS] = &rcs;
        commandStreamerHelperTable[EngineType::ENGINE_BCS] = &bcs;
        commandStreamerHelperTable[EngineType::ENGINE_VCS] = &vcs;
        commandStreamerHelperTable[EngineType::ENGINE_VECS] = &vecs;
    }
} populateGen8;

CommandStreamerHelper &GpuGen8::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device == 0);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}

} // namespace aub_stream
