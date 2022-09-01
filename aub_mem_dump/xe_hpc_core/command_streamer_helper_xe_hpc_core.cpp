/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hpc_core/command_streamer_helper_xe_hpc_core.h"
#include "aub_mem_dump/page_table_pml5.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), //CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), //IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), //MI_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00007014),
        MMIOPair(mmioEngine + 0x000024d4, 0x0000e48c),
        MMIOPair(mmioEngine + 0x000024d8, 0x0000e18c),
        MMIOPair(mmioEngine + 0x000024dc, 0x00004de0),
        MMIOPair(mmioEngine + 0x000024e0, 0x00004de4),
        MMIOPair(mmioEngine + 0x000024e4, 0x0000f180),
        MMIOPair(mmioEngine + 0x000024e8, 0x0000e194),
        MMIOPair(mmioEngine + 0x000024ec, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f0, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f4, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f8, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024fc, 0x0000e000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), //CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), //IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), //MI_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00007014),
        MMIOPair(mmioEngine + 0x000024d4, 0x0000e48c),
        MMIOPair(mmioEngine + 0x000024d8, 0x0000e18c),
        MMIOPair(mmioEngine + 0x000024dc, 0x00004de0),
        MMIOPair(mmioEngine + 0x000024e0, 0x00004de4),
        MMIOPair(mmioEngine + 0x000024e4, 0x0000f180),
        MMIOPair(mmioEngine + 0x000024e8, 0x0000e194),
        MMIOPair(mmioEngine + 0x000024ec, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f0, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f4, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f8, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024fc, 0x0000e000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), //MI_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00007014),
        MMIOPair(mmioEngine + 0x000024d4, 0x0000e48c),
        MMIOPair(mmioEngine + 0x000024d8, 0x0000e18c),
        MMIOPair(mmioEngine + 0x000024dc, 0x00004de0),
        MMIOPair(mmioEngine + 0x000024e0, 0x00004de4),
        MMIOPair(mmioEngine + 0x000024e4, 0x0000f180),
        MMIOPair(mmioEngine + 0x000024e8, 0x0000e194),
        MMIOPair(mmioEngine + 0x000024ec, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f0, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f4, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024f8, 0x0000e000),
        MMIOPair(mmioEngine + 0x000024fc, 0x0000e000),
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), //GFX_MODE
    };

    return engineMMIO;
}

static CommandStreamerHelperXeHpcCore<CommandStreamerHelperRcs> rcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperBcs> bcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperVcs> vcsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperVecs> vecsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs> ccs0Devices[GpuXeHpCore::numSupportedDevices] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs> ccs1Devices[GpuXeHpCore::numSupportedDevices] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs> ccs2Devices[GpuXeHpCore::numSupportedDevices] = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs> ccs3Devices[GpuXeHpCore::numSupportedDevices] = {{0, 3}, {1, 3}, {2, 3}, {3, 3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperCccs> cccsDevices[GpuXeHpCore::numSupportedDevices] = {{0}, {1}, {2}, {3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs1Devices[GpuXeHpCore::numSupportedDevices] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs2Devices[GpuXeHpCore::numSupportedDevices] = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs3Devices[GpuXeHpCore::numSupportedDevices] = {{0, 3}, {1, 3}, {2, 3}, {3, 3}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs4Devices[GpuXeHpCore::numSupportedDevices] = {{0, 4}, {1, 4}, {2, 4}, {3, 4}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs5Devices[GpuXeHpCore::numSupportedDevices] = {{0, 5}, {1, 5}, {2, 5}, {3, 5}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs6Devices[GpuXeHpCore::numSupportedDevices] = {{0, 6}, {1, 6}, {2, 6}, {3, 6}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs7Devices[GpuXeHpCore::numSupportedDevices] = {{0, 7}, {1, 7}, {2, 7}, {3, 7}};
static CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs> bcs8Devices[GpuXeHpCore::numSupportedDevices] = {{0, 8}, {1, 8}, {2, 8}, {3, 8}};

static CommandStreamerHelper *commandStreamerHelperTable[GpuXeHpcCore::numSupportedDevices][EngineType::NUM_ENGINES] = {};

struct PopulateXeHpcCore {
    PopulateXeHpcCore() {
        auto fillEngine = [](EngineType engineType, CommandStreamerHelper *csHelper) {
            for (uint32_t i = 0; i < GpuXeHpcCore::numSupportedDevices; i++) {
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
        fillEngine(EngineType::ENGINE_CCCS, cccsDevices);
        fillEngine(EngineType::ENGINE_BCS1, bcs1Devices);
        fillEngine(EngineType::ENGINE_BCS2, bcs2Devices);
        fillEngine(EngineType::ENGINE_BCS3, bcs3Devices);
        fillEngine(EngineType::ENGINE_BCS4, bcs4Devices);
        fillEngine(EngineType::ENGINE_BCS5, bcs5Devices);
        fillEngine(EngineType::ENGINE_BCS6, bcs6Devices);
        fillEngine(EngineType::ENGINE_BCS7, bcs7Devices);
        fillEngine(EngineType::ENGINE_BCS8, bcs8Devices);
    }
} populateXeHpcCore;

CommandStreamerHelper &GpuXeHpcCore::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    assert(device < GpuXeHpcCore::numSupportedDevices);
    assert(isEngineSupported(engineType));
    auto csh = commandStreamerHelperTable[device][engineType];
    assert(csh);
    csh->gpu = this;
    return *csh;
}

PageTable *GpuXeHpcCore::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    if (gpuAddressSpace == ((1ull << 57) - 1)) {
        return new PML5(*this, physicalAddressAllocator, memoryBank);
    }
    return new PML4(*this, physicalAddressAllocator, memoryBank);
}

} // namespace aub_stream
