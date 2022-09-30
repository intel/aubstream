/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpg_core/command_streamer_helper_xe_hpg_core.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), //CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), //IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), //MI_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x40002580),
        MMIOPair(mmioEngine + 0x000024d4, 0x00007014),
        MMIOPair(mmioEngine + 0x000024d8, 0x00007304),
        MMIOPair(mmioEngine + 0x000024dc, 0x0000d924),
        MMIOPair(mmioEngine + 0x000024e0, 0x103860e8),
        MMIOPair(mmioEngine + 0x000024e4, 0x10386260),
        MMIOPair(mmioEngine + 0x000024e8, 0x10386670),
        MMIOPair(mmioEngine + 0x000024ec, 0x20006100),
        MMIOPair(mmioEngine + 0x000024f0, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f4, 0x00000000),
        MMIOPair(mmioEngine + 0x000024f8, 0x00000000),
        MMIOPair(mmioEngine + 0x000024fc, 0x00000000),

    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        MMIOPair(mmioEngine + 0x000024d0, 0x103860e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10386260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10386670),
        MMIOPair(mmioEngine + 0x000024dc, 0x10386674),
        MMIOPair(mmioEngine + 0x000024e0, 0x10386678),
        MMIOPair(mmioEngine + 0x000024e4, 0x10386264),
        MMIOPair(mmioEngine + 0x000024e8, 0x10386268),
        MMIOPair(mmioEngine + 0x000024ec, 0x10386861),
        MMIOPair(mmioEngine + 0x000024f0, 0x10386871),
        MMIOPair(mmioEngine + 0x000024f4, 0x10386881),
        MMIOPair(mmioEngine + 0x000024f8, 0x10386891),
        MMIOPair(mmioEngine + 0x000024fc, 0x103868a1),

        MMIOPair(mmioEngine + 0x00002010, 0x103868b1),
        MMIOPair(mmioEngine + 0x00002014, 0x103868c1),
        MMIOPair(mmioEngine + 0x00002018, 0x103868d1),
        MMIOPair(mmioEngine + 0x0000201c, 0x00000000),

        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        MMIOPair(mmioEngine + 0x000024d0, 0x103860e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10386260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10386670),
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
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), //MI_MODE

        // FORCE_TO_NONPRIV
        // No CCS passlists in driver currently...
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false);

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpgCore<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false);

    return engineMMIO;
}

static CommandStreamerHelperXeHpgCore<CommandStreamerHelperRcs> rcsDevices[GpuXeHpgCore::numSupportedDevices] = {{0}};
static CommandStreamerHelperXeHpgCore<CommandStreamerHelperBcs> bcsDevices[GpuXeHpgCore::numSupportedDevices] = {{0}};
static CommandStreamerHelperXeHpgCore<CommandStreamerHelperCcs> ccs0Devices[GpuXeHpgCore::numSupportedDevices] = {{0, 0}};
static CommandStreamerHelperXeHpgCore<CommandStreamerHelperCcs> ccs1Devices[GpuXeHpgCore::numSupportedDevices] = {{0, 1}};
static CommandStreamerHelperXeHpgCore<CommandStreamerHelperCcs> ccs2Devices[GpuXeHpgCore::numSupportedDevices] = {{0, 2}};
static CommandStreamerHelperXeHpgCore<CommandStreamerHelperCcs> ccs3Devices[GpuXeHpgCore::numSupportedDevices] = {{0, 3}};

static CommandStreamerHelper *commandStreamerHelperTable[GpuXeHpgCore::numSupportedDevices][EngineType::NUM_ENGINES] = {};

struct PopulateXeHpgCore {
    PopulateXeHpgCore() {
        auto fillEngine = [](EngineType engineType, CommandStreamerHelper *csHelper) {
            for (uint32_t i = 0; i < GpuXeHpgCore::numSupportedDevices; i++) {
                commandStreamerHelperTable[i][engineType] = &csHelper[i];
            }
        };

        fillEngine(EngineType::ENGINE_RCS, rcsDevices);
        fillEngine(EngineType::ENGINE_BCS, bcsDevices);
        fillEngine(EngineType::ENGINE_CCS, ccs0Devices);
        fillEngine(EngineType::ENGINE_CCS1, ccs1Devices);
        fillEngine(EngineType::ENGINE_CCS2, ccs2Devices);
        fillEngine(EngineType::ENGINE_CCS3, ccs3Devices);
    }
} populateXeHpgCore;

CommandStreamerHelper &GpuXeHpgCore::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device < GpuXeHpgCore::numSupportedDevices);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[device][engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}
void GpuXeHpgCore::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const {
    if ((stream.getStreamMode() == aub_stream::mode::tbx || stream.getStreamMode() == aub_stream::mode::aubFileAndTbx || stream.getStreamMode() == aub_stream::mode::tbxShm)) {
        auto flatCcsSize = memoryBankSize / 256;
        for (uint32_t i = 0; i < devicesCount; i++) {
            // put flat ccs at the end of memory bank (minus 9MB)
            uint64_t flatCcsBaseAddr = memoryBankSize * (i + 1);
            flatCcsBaseAddr -= flatCcsSize;
            flatCcsBaseAddr -= 9 * MB; // // wopcm and ggtt - match aub file mode
            initializeFlatCcsBaseAddressMmio(stream, i, flatCcsBaseAddr);
        }
    }
}
} // namespace aub_stream
