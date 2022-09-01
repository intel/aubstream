/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hp_core/command_streamer_helper_xe_hp_core.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), //CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), //IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x10032a83),
        MMIOPair(mmioEngine + 0x000024d4, 0x10032264),
        MMIOPair(mmioEngine + 0x000024d8, 0x10032268),
        MMIOPair(mmioEngine + 0x000024dc, 0x00007014),
        MMIOPair(mmioEngine + 0x000024e0, 0x20006100),
        MMIOPair(mmioEngine + 0x000024e4, 0x0000d924),
        MMIOPair(mmioEngine + 0x000024e8, 0x00002248),
        MMIOPair(mmioEngine + 0x000024ec, 0x10032883),
        MMIOPair(mmioEngine + 0x000024f0, 0x10032861),
        MMIOPair(mmioEngine + 0x000024f4, 0x10032678),
        MMIOPair(mmioEngine + 0x000024f8, 0x00007010),
        MMIOPair(mmioEngine + 0x000024fc, 0x0000db1c),
        MMIOPair(mmioEngine + 0x00002010, 0x00007304),
        MMIOPair(mmioEngine + 0x00002014, 0x10032674),
        MMIOPair(mmioEngine + 0x00002018, 0x10032b83),
        MMIOPair(mmioEngine + 0x0000201c, 0x10032871),
        MMIOPair(mmioEngine + 0x000021e0, 0x10032983),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x100320e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10032260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10032670),
        MMIOPair(mmioEngine + 0x000024dc, 0x10032883),
        MMIOPair(mmioEngine + 0x000024e0, 0x10032861),
        MMIOPair(mmioEngine + 0x000024e4, 0x10032871),
        MMIOPair(mmioEngine + 0x000024e8, 0x10032891),
        MMIOPair(mmioEngine + 0x000024ec, 0x100328a1),
        MMIOPair(mmioEngine + 0x000024f0, 0x100328b1),
        MMIOPair(mmioEngine + 0x000024f4, 0x100328c1),
        MMIOPair(mmioEngine + 0x000024f8, 0x100328d1),
        MMIOPair(mmioEngine + 0x000024fc, 0x10032983),
        MMIOPair(mmioEngine + 0x00002010, 0x10032a83),
        MMIOPair(mmioEngine + 0x00002014, 0x10032b83),
        MMIOPair(mmioEngine + 0x00002018, 0x10032264),
        MMIOPair(mmioEngine + 0x0000201c, 0x10032268),
        MMIOPair(mmioEngine + 0x000021e0, 0x10032674),
        MMIOPair(mmioEngine + 0x000021e4, 0x10032678),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),

    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x100320e8),
        MMIOPair(mmioEngine + 0x000024d4, 0x10032260),
        MMIOPair(mmioEngine + 0x000024d8, 0x10032670),
        MMIOPair(mmioEngine + 0x000024dc, 0x10032883),
        MMIOPair(mmioEngine + 0x000024e0, 0x10032861),
        MMIOPair(mmioEngine + 0x000024e4, 0x10032871),
        MMIOPair(mmioEngine + 0x000024e8, 0x10032983),
        MMIOPair(mmioEngine + 0x000024ec, 0x10032a83),
        MMIOPair(mmioEngine + 0x000024f0, 0x10032b83),
        MMIOPair(mmioEngine + 0x000024f4, 0x10032264),
        MMIOPair(mmioEngine + 0x000024f8, 0x10032268),
        MMIOPair(mmioEngine + 0x000024fc, 0x10032674),
        MMIOPair(mmioEngine + 0x00002010, 0x10032678),
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
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x0001a0c8),
        MMIOPair(mmioEngine + 0x000024d4, 0x0000d922),
        MMIOPair(mmioEngine + 0x000024d8, 0x0000da12),
        MMIOPair(mmioEngine + 0x000024dc, 0x0000db1c),
        MMIOPair(mmioEngine + 0x000024e0, 0x10032264),
        MMIOPair(mmioEngine + 0x000024e4, 0x10032268),
        MMIOPair(mmioEngine + 0x000024e8, 0x10032674),
        MMIOPair(mmioEngine + 0x000024ec, 0x10032678),
        MMIOPair(mmioEngine + 0x000024f0, 0x10032861),
        MMIOPair(mmioEngine + 0x000024f4, 0x10032871),
        MMIOPair(mmioEngine + 0x000024f8, 0x10032883),
        MMIOPair(mmioEngine + 0x000024fc, 0x10032983),
        MMIOPair(mmioEngine + 0x00002010, 0x10032a83),
        MMIOPair(mmioEngine + 0x00002014, 0x10032b83),
        MMIOPair(mmioEngine + 0x00002018, 0x00002248),
        MMIOPair(mmioEngine + 0x0000201c, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e0, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e4, 0x00000000),
        MMIOPair(mmioEngine + 0x000021e8, 0x00000000),
        MMIOPair(mmioEngine + 0x000021ec, 0x00000000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false); // not supported on XEHP

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false); // not supported on XEHP

    return engineMMIO;
}

static CommandStreamerHelperXeHpCore<CommandStreamerHelperRcs> rcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperBcs> bcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperVcs> vcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperVecs> vecsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperCcs> ccs0Devices[GpuXeHpCore::numSupportedDevices] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperCcs> ccs1Devices[GpuXeHpCore::numSupportedDevices] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperCcs> ccs2Devices[GpuXeHpCore::numSupportedDevices] = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
static CommandStreamerHelperXeHpCore<CommandStreamerHelperCcs> ccs3Devices[GpuXeHpCore::numSupportedDevices] = {{0, 3}, {1, 3}, {2, 3}, {3, 3}};

static CommandStreamerHelper *commandStreamerHelperTable[GpuXeHpCore::numSupportedDevices][EngineType::NUM_ENGINES] = {};

struct PopulateXeHpCore {
    PopulateXeHpCore() {
        auto fillEngine = [](EngineType engineType, CommandStreamerHelper *csHelper) {
            for (uint32_t i = 0; i < GpuXeHpCore::numSupportedDevices; i++) {
                commandStreamerHelperTable[i][engineType] = &csHelper[i];
            }
        };

        fillEngine(EngineType::ENGINE_RCS, rcsDevices);
        fillEngine(EngineType::ENGINE_BCS, bcsDevices);
        fillEngine(EngineType::ENGINE_VCS, vcsDevices);
        fillEngine(EngineType::ENGINE_VECS, vecsDevices);
        fillEngine(EngineType::ENGINE_CCS, ccs0Devices);
        fillEngine(EngineType::ENGINE_CCS1, ccs1Devices);
        fillEngine(EngineType::ENGINE_CCS2, ccs2Devices);
        fillEngine(EngineType::ENGINE_CCS3, ccs3Devices);
    }
} populateXeHpCore;

CommandStreamerHelper &GpuXeHpCore::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device < GpuXeHpCore::numSupportedDevices);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[device][engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}

void GpuXeHpCore::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const {
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
