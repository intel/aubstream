/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/gen12lp/command_streamer_helper_gen12lp.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperGen12LP<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), //CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), //IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00007014),
        MMIOPair(mmioEngine + 0x000024d4, 0x00007304),
        MMIOPair(mmioEngine + 0x000024d8, 0x000020c8),
        MMIOPair(mmioEngine + 0x000024dc, 0x0000d924),
        MMIOPair(mmioEngine + 0x000024e0, 0x10002351),
        MMIOPair(mmioEngine + 0x000024e4, 0x10002341),
        MMIOPair(mmioEngine + 0x000024e8, 0x100320e8),
        MMIOPair(mmioEngine + 0x000024ec, 0x10032260),
        MMIOPair(mmioEngine + 0x000024f0, 0x10032670),
        MMIOPair(mmioEngine + 0x000024f4, 0x000028a0),
        MMIOPair(mmioEngine + 0x000024f8, 0x00002754),
        MMIOPair(mmioEngine + 0x000024fc, 0x00007010),
        MMIOPair(mmioEngine + 0x00002010, 0x0000d922),
        MMIOPair(mmioEngine + 0x00002014, 0x0000da12),
        MMIOPair(mmioEngine + 0x00002018, 0x0000db1c),
        MMIOPair(mmioEngine + 0x0000201c, 0x20006100),
        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),

    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen12LP<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen12LP<CommandStreamerHelperVcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x100320e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10032260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10032670),
        MMIOPair(mmioEngine + 0x000024dc, 0x10032881),
        MMIOPair(mmioEngine + 0x000024e0, 0x10032861),
        MMIOPair(mmioEngine + 0x000024e4, 0x10032871),
        MMIOPair(mmioEngine + 0x000024e8, 0x10032891),
        MMIOPair(mmioEngine + 0x000024ec, 0x100328a1),
        MMIOPair(mmioEngine + 0x000024f0, 0x100328b1),
        MMIOPair(mmioEngine + 0x000024f4, 0x100328c1),
        MMIOPair(mmioEngine + 0x000024f8, 0x100328d1),
        MMIOPair(mmioEngine + 0x000024fc, 0x00000000),
        MMIOPair(mmioEngine + 0x00002010, 0x00000000),
        MMIOPair(mmioEngine + 0x00002014, 0x00000000),
        MMIOPair(mmioEngine + 0x00002018, 0x00000000),
        MMIOPair(mmioEngine + 0x0000201c, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen12LP<CommandStreamerHelperVecs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x100320e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10032260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10032670),
        MMIOPair(mmioEngine + 0x000024dc, 0x00000000),
        MMIOPair(mmioEngine + 0x000024e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000024e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000024e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000024ec, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f0, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f4, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f8, 0x00000000),
        MMIOPair(mmioEngine + 0x000024fc, 0x00000000),
        MMIOPair(mmioEngine + 0x00002010, 0x00000000),
        MMIOPair(mmioEngine + 0x00002014, 0x00000000),
        MMIOPair(mmioEngine + 0x00002018, 0x00000000),
        MMIOPair(mmioEngine + 0x0000201c, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperGen12LP<CommandStreamerHelperCcs>::getEngineMMIO() const {
    MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x0001a0c8),
        MMIOPair(mmioEngine + 0x000024d4, 0x0000d922),
        MMIOPair(mmioEngine + 0x000024d8, 0x0000da12),
        MMIOPair(mmioEngine + 0x000024dc, 0x0000db1c),
        MMIOPair(mmioEngine + 0x000024e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000024e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000024e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000024ec, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f0, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f4, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f8, 0x00000000),
        MMIOPair(mmioEngine + 0x000024fc, 0x00000000),
        MMIOPair(mmioEngine + 0x00002010, 0x00000000),
        MMIOPair(mmioEngine + 0x00002014, 0x00000000),
        MMIOPair(mmioEngine + 0x00002018, 0x00000000),
        MMIOPair(mmioEngine + 0x0000201c, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

static CommandStreamerHelperGen12LP<CommandStreamerHelperRcs> rcs(0);
static CommandStreamerHelperGen12LP<CommandStreamerHelperBcs> bcs(0);
static CommandStreamerHelperGen12LP<CommandStreamerHelperVcs> vcs(0);
static CommandStreamerHelperGen12LP<CommandStreamerHelperVecs> vecs(0);
static CommandStreamerHelperGen12LP<CommandStreamerHelperCcs> ccs(0, 0);

static CommandStreamerHelper *commandStreamerHelperTable[EngineType::NUM_ENGINES] = {};

struct PopulateGen12LP {
    PopulateGen12LP() {
        commandStreamerHelperTable[EngineType::ENGINE_RCS] = &rcs;
        commandStreamerHelperTable[EngineType::ENGINE_BCS] = &bcs;
        commandStreamerHelperTable[EngineType::ENGINE_VCS] = &vcs;
        commandStreamerHelperTable[EngineType::ENGINE_VECS] = &vecs;
        commandStreamerHelperTable[EngineType::ENGINE_CCS] = &ccs;
    }
} populateGen12LP;

CommandStreamerHelper &
GpuGen12LP::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device == 0);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}

} // namespace aub_stream
