/*
 * Copyright (C) 2022-2025 Intel Corporation
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
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE

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
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE

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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE

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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

GpuXeHpcCore::GpuXeHpcCore() {
    for (auto deviceId = 0u; deviceId < numSupportedDevices; deviceId++) {
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCCS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperCccs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperRcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperBcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperVcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperVecs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>>(deviceId, 0);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS1] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>>(deviceId, 1);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS2] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>>(deviceId, 2);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS3] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperCcs>>(deviceId, 3);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS1] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 1);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS2] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 2);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS3] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 3);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS4] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 4);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS5] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 5);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS6] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 6);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS7] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 7);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS8] = std::make_unique<CommandStreamerHelperXeHpcCore<CommandStreamerHelperLinkBcs>>(deviceId, 8);
    }
}

PageTable *GpuXeHpcCore::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    if (gpuAddressSpace == ((1ull << 57) - 1)) {
        return new PML5(*this, physicalAddressAllocator, memoryBank);
    }
    return new PML4(*this, physicalAddressAllocator, memoryBank);
}

CommandStreamerHelper &GpuXeHpcCore::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    auto &csh = commandStreamerHelperTable[device][engineType];
    csh->gpu = this;
    return *csh;
}

const std::vector<EngineType> GpuXeHpcCore::getSupportedEngines() const {
    static constexpr std::array<EngineType, 17> engines = {{ENGINE_BCS, ENGINE_VCS, ENGINE_VECS,
                                                            ENGINE_CCS, ENGINE_CCS1, ENGINE_CCS2, ENGINE_CCS3, ENGINE_CCCS,
                                                            ENGINE_BCS1, ENGINE_BCS2, ENGINE_BCS3, ENGINE_BCS4, ENGINE_BCS5,
                                                            ENGINE_BCS6, ENGINE_BCS7, ENGINE_BCS8}};
    return std::vector<EngineType>(engines.begin(), engines.end());
}

constexpr uint32_t GpuXeHpcCore::getPatIndexMmioAddr(uint32_t index) {
    assert(index <= 7);

    return 0x4800 + (index * 4);
}

const MMIOList GpuXeHpcCore::getGlobalMMIO() const {
    const MMIOList globalMMIO = {
        MMIOPair(0x00002090, 0xffff0000), // CHICKEN_PWR_CTX_RASTER_1
        MMIOPair(0x000020d8, 0x00020000), // CS_DEBUG_MODE2_RCSUNIT
        MMIOPair(0x000020e0, 0xffff4000), // FF_SLICE_CS_CHICKEN1_RCSUNIT
        MMIOPair(0x000020e4, 0xffff0000), // FF_SLICE_CS_CHICKEN2_RCSUNIT
        MMIOPair(0x000020ec, 0xffff0051), // CS_DEBUG_MODE1
        MMIOPair(0x00002580, 0xffff0005), // CS_CHICKEN1

        // GLOBAL_MOCS
        // bits: 8: IG_PAT, 7-6: L3_CLOS, 5-4: L3_CACHE_POLICY, 3-2:L4_CACHE_POLICY, 1-0:0
        MMIOPair(0x00004000, 0b0000'0000'1100), // IG_PAT (PAT)  L3_CLOS (0), L3 (WB), L4 (UC)
        MMIOPair(0x00004004, 0b0001'0000'1100), // IG_PAT (MOCS) L3_CLOS (0), L3 (WB), L4 (UC)
        MMIOPair(0x00004008, 0b0001'0011'0000), // IG_PAT (MOCS) L3_CLOS (0), L3 (UC), L4 (WB)
        MMIOPair(0x0000400C, 0b0001'0011'1100), // IG_PAT (MOCS) L3_CLOS (0), L3 (UC), L4 (UC)
        MMIOPair(0x00004010, 0b0001'0000'0000), // IG_PAT (MOCS) L3_CLOS (0), L3 (WB), L4 (WB)
        MMIOPair(0x00004014, 0x00060038),
        MMIOPair(0x00004018, 0x00000000),
        MMIOPair(0x0000401C, 0x00000033),
        MMIOPair(0x00004020, 0x00060037),
        MMIOPair(0x00004024, 0x0000003B),
        MMIOPair(0x00004028, 0x00000032),
        MMIOPair(0x0000402C, 0x00000036),
        MMIOPair(0x00004030, 0x0000003A),
        MMIOPair(0x00004034, 0x00000033),
        MMIOPair(0x00004038, 0x00000037),
        MMIOPair(0x0000403C, 0x0000003B),
        MMIOPair(0x00004040, 0x00000030),
        MMIOPair(0x00004044, 0x00000034),
        MMIOPair(0x00004048, 0x00000038),
        MMIOPair(0x0000404C, 0x00000031),
        MMIOPair(0x00004050, 0x00000032),
        MMIOPair(0x00004054, 0x00000036),
        MMIOPair(0x00004058, 0x0000003A),
        MMIOPair(0x0000405C, 0x00000033),
        MMIOPair(0x00004060, 0x00000037),
        MMIOPair(0x00004064, 0x0000003B),
        MMIOPair(0x00004068, 0x00000032),
        MMIOPair(0x0000406C, 0x00000036),
        MMIOPair(0x00004070, 0x0000003A),
        MMIOPair(0x00004074, 0x00000033),
        MMIOPair(0x00004078, 0x00000037),
        MMIOPair(0x0000407C, 0x0000003B),
        MMIOPair(0x00004080, 0x00000030),
        MMIOPair(0x00004084, 0x00000034),
        MMIOPair(0x00004088, 0x00000038),
        MMIOPair(0x0000408C, 0x00000031),
        MMIOPair(0x00004090, 0x00000032),
        MMIOPair(0x00004094, 0x00000036),
        MMIOPair(0x00004098, 0x0000003A),
        MMIOPair(0x0000409C, 0x00000033),
        MMIOPair(0x000040A0, 0x00000037),
        MMIOPair(0x000040A4, 0x0000003B),
        MMIOPair(0x000040A8, 0x00000032),
        MMIOPair(0x000040AC, 0x00000036),
        MMIOPair(0x000040B0, 0x0000003A),
        MMIOPair(0x000040B4, 0x00000033),
        MMIOPair(0x000040B8, 0x00000037),
        MMIOPair(0x000040BC, 0x0000003B),
        MMIOPair(0x000040C0, 0x00000038),
        MMIOPair(0x000040C4, 0x00000034),
        MMIOPair(0x000040C8, 0x00000038),
        MMIOPair(0x000040CC, 0x00000031),
        MMIOPair(0x000040D0, 0x00000032),
        MMIOPair(0x000040D4, 0x00000036),
        MMIOPair(0x000040D8, 0x0000003A),
        MMIOPair(0x000040DC, 0x00000033),
        MMIOPair(0x000040E0, 0x00000037),
        MMIOPair(0x000040E4, 0x0000003B),
        MMIOPair(0x000040E8, 0x00000032),
        MMIOPair(0x000040EC, 0x00000036),
        MMIOPair(0x000040F0, 0x00000038),
        MMIOPair(0x000040F4, 0x00000038),
        MMIOPair(0x000040F8, 0x00000038),
        MMIOPair(0x000040FC, 0x00000038),

        // LNCF_MOCS (PVC, ...)
        MMIOPair(0x0000B020, 0x00100000), // LNCF0: Lower - Error (Reserved for Non-Use),  Upper - UC
        MMIOPair(0x0000B024, 0x00500030), // LNCF1: Lower - L3
        MMIOPair(0x0000B028, 0x00B00010),
        MMIOPair(0x0000B02C, 0x00000000),
        MMIOPair(0x0000B030, 0x0030001F),
        MMIOPair(0x0000B034, 0x00170013),
        MMIOPair(0x0000B038, 0x0000001F),
        MMIOPair(0x0000B03C, 0x00000000),
        MMIOPair(0x0000B040, 0x00100000),
        MMIOPair(0x0000B044, 0x00170013),
        MMIOPair(0x0000B048, 0x0010001F),
        MMIOPair(0x0000B04C, 0x00170013),
        MMIOPair(0x0000B050, 0x0030001F),
        MMIOPair(0x0000B054, 0x00170013),
        MMIOPair(0x0000B058, 0x0000001F),
        MMIOPair(0x0000B05C, 0x00000000),
        MMIOPair(0x0000B060, 0x00100000),
        MMIOPair(0x0000B064, 0x00170013),
        MMIOPair(0x0000B068, 0x0010001F),
        MMIOPair(0x0000B06C, 0x00170013),
        MMIOPair(0x0000B070, 0x0030001F),
        MMIOPair(0x0000B074, 0x00170013),
        MMIOPair(0x0000B078, 0x0000001F),
        MMIOPair(0x0000B07C, 0x00000000),
        MMIOPair(0x0000B080, 0x009000B0),
        MMIOPair(0x0000B084, 0x00170013),
        MMIOPair(0x0000B088, 0x0010001F),
        MMIOPair(0x0000B08C, 0x00170013),
        MMIOPair(0x0000B090, 0x0030001F),
        MMIOPair(0x0000B094, 0x00170013),
        MMIOPair(0x0000B098, 0x00100010),
        MMIOPair(0x0000B09C, 0x00100010),

        // PAT_INDEX
        MMIOPair(0x00004100, 0x0000000),
        MMIOPair(0x00004104, 0x0000000),
        MMIOPair(0x00004108, 0x0000000),
        MMIOPair(0x0000410c, 0x0000000),
        MMIOPair(0x00004110, 0x0000000),
        MMIOPair(0x00004114, 0x0000000),
        MMIOPair(0x00004118, 0x0000000),
        MMIOPair(0x0000411c, 0x0000000),

        MMIOPair(0x00004b80, 0xffff1001), // GACB_PERF_CTRL_REG
        MMIOPair(0x00007000, 0xffff0000), // CACHE_MODE_0
        MMIOPair(0x00007004, 0xffff0000), // CACHE_MODE_1
        MMIOPair(0x00009008, 0x00000200), // IDICR
        MMIOPair(0x0000900c, 0x00001b40), // SNPCR
        MMIOPair(0x0000b120, 0x14000002), // LTCDREG
        MMIOPair(0x0000b134, 0xa0000000), // L3ALLOCREG
        MMIOPair(0x0000b234, 0xa0000000), // L3ALLOCREG_CCS0
        MMIOPair(0x0000ce90, 0x00030003), // GFX_MULT_CTXT_CTL
        MMIOPair(0x0000cf58, 0x80000000), // LMEM_CFG for local memory
        MMIOPair(0x0000e194, 0xffff0002), // CHICKEN_SAMPLER_2
        MMIOPair(0x00014800, 0x00030003), // RCU_MODE
        MMIOPair(0x00014804, 0x0fff0000), // CCS_MODE
        MMIOPair(0x0001a0d8, 0x00020000), // CS_DEBUG_MODE2_CCSUNIT
        MMIOPair(0x00042080, 0x00000000), // CHICKEN_MISC_1

        // bits: 3-2: CLOS, 1-0: MEM_TYPE
        MMIOPair(getPatIndexMmioAddr(0), 0b0000), // UC
        MMIOPair(getPatIndexMmioAddr(1), 0b0001), // WC
        MMIOPair(getPatIndexMmioAddr(2), 0b0010), // WT
        MMIOPair(getPatIndexMmioAddr(3), 0b0011), // WB
        MMIOPair(getPatIndexMmioAddr(4), 0b0110), // WT (CLOS1)
        MMIOPair(getPatIndexMmioAddr(5), 0b0111), // WB (CLOS1)
        MMIOPair(getPatIndexMmioAddr(6), 0b1010), // WT (CLOS2)
        MMIOPair(getPatIndexMmioAddr(7), 0b1011), // WT (CLOS2)
    };
    return globalMMIO;
}

uint64_t GpuXeHpcCore::getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const {
    auto bits = toBitValue(PpgttEntryBits::atomicEnableBit);

    bits |= allocationParams.uncached ? patIndex0 : patIndex3;

    return bits;
}

} // namespace aub_stream
