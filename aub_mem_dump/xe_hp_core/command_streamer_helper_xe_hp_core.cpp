/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_hp_core/command_streamer_helper_xe_hp_core.h"
#include "aub_mem_dump/align_helpers.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeHpCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

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
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

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

void GpuXeHpCore::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {
    if (IsAnyTbxMode(stream.getStreamMode())) {
        for (uint32_t i = 0; i < devicesCount; i++) {
            // put flat ccs at the beginning of stolen memory

            uint64_t flatCcsBaseAddr = stolenMemory.getBaseAddress(i);
            initializeFlatCcsBaseAddressMmio(stream, i, flatCcsBaseAddr);
        }
    }
}
const std::vector<EngineType> GpuXeHpCore::getSupportedEngines() const {
    static constexpr std::array<EngineType, 9> engines = {{ENGINE_RCS, ENGINE_BCS, ENGINE_VCS, ENGINE_VECS,
                                                           ENGINE_CCS, ENGINE_CCS1, ENGINE_CCS2, ENGINE_CCS3}};
    return std::vector<EngineType>(engines.begin(), engines.end());
}

const MMIOList GpuXeHpCore::getGlobalMMIO() const {
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

        // LNCF_MOCS (DG2, MTL)
        MMIOPair(0x0000B020, 0x00D00090), // LNCF0 - UC (Coherent; GO:L3),                Upper: UC (Coherent; GO:Memory)
        MMIOPair(0x0000B024, 0x00B00050), // LNCF1 - UC (Non-Coherent*; GO:Memory),       Upper: L3
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
        MMIOPair(0x0001a0d8, 0x00020000), // CS_DEBUG_MODE2_CCSUNIT
        MMIOPair(0x00042080, 0x00000000), // CHICKEN_MISC_1
    };
    return globalMMIO;
}

void GpuXeHpCore::initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const {
    uint32_t mmioDevice = 0;

    for (uint32_t device = 0; device < devicesCount; device++) {
        const auto &globalMMIO = getGlobalMMIO();
        for (const auto &mmioPair : globalMMIO) {
            stream.writeMMIO(mmioDevice + mmioPair.first, mmioPair.second);
        }

        mmioDevice += mmioDeviceOffset;
    }
}

bool GpuXeHpCore::isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const {
    assert(MEMORY_BANK_SYSTEM == 0);
    auto supportedBanks = MEMORY_BANK_SYSTEM | MEMORY_BANK_0 | MEMORY_BANK_1 | MEMORY_BANK_2 | MEMORY_BANK_3;
    auto unsupportedBanks = MEMORY_BANKS_ALL ^ supportedBanks;

    if (unsupportedBanks & memoryBanks) {
        return false;
    }

    // Local MEMORY_BANKs
    if (memoryBanks & supportedBanks) {
        return alignment == 65536;
    }

    // MEMORY_BANK_SYSTEM
    return alignment == 4096 || alignment == 65536;
}

void GpuXeHpCore::setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const {
    assert(deviceCount > 0u);
    assert(deviceCount <= this->deviceCount);

    if (1 == deviceCount) {
        return;
    }

    auto gb = memoryBankSize / GB;
    assert(gb > 0);
    assert(gb < 128);

    uint64_t base = 0u;
    uint32_t offset = 0x4900;
    for (auto device = 0u; device < deviceCount; ++device) {
        uint32_t value = 0;
        value |= gb << 8;
        value |= base << 1;
        value |= 1;

        stream.writeMMIO(offset, value);
        base += gb;
        offset += 4;
    }
}

void GpuXeHpCore::setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {
    assert(deviceCount > 0u);
    assert(deviceCount <= this->deviceCount);

    const uint32_t mmioDevice[4] = {0, 16 * MB, 32 * MB, 48 * MB};
    const uint32_t gsmBase = 0x108100;
    const uint32_t gsmBaseRem1 = 0x108108;
    const uint32_t gsmBaseRem2 = 0x108110;
    const uint32_t gsmBaseRem3 = 0x108118;

    for (auto device = 0u; device < deviceCount; ++device) {
        uint64_t gttBase = getGGTTBaseAddress(device, memoryBankSize, stolenMemory.getBaseAddress(device));
        stream.writeMMIO(mmioDevice[device] + gsmBase + 4, static_cast<uint32_t>(gttBase >> 32));
        stream.writeMMIO(mmioDevice[device] + gsmBase + 0, static_cast<uint32_t>(gttBase & 0xFFF00000));

        if (1 == device) {
            uint64_t gttBaseRem1 = getGGTTBaseAddress(1, memoryBankSize, stolenMemory.getBaseAddress(device));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem1 + 4, static_cast<uint32_t>(gttBaseRem1 >> 32));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem1 + 0, static_cast<uint32_t>(gttBaseRem1 & 0xFFF00000));
        }
        if (2 == device) {
            uint64_t gttBaseRem2 = getGGTTBaseAddress(2, memoryBankSize, stolenMemory.getBaseAddress(device));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem2 + 4, static_cast<uint32_t>(gttBaseRem2 >> 32));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem2 + 0, static_cast<uint32_t>(gttBaseRem2 & 0xFFF00000));
        }
        if (3 == device) {
            uint64_t gttBaseRem3 = getGGTTBaseAddress(3, memoryBankSize, stolenMemory.getBaseAddress(device));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem3 + 4, static_cast<uint32_t>(gttBaseRem3 >> 32));
            stream.writeMMIO(mmioDevice[0] + gsmBaseRem3 + 0, static_cast<uint32_t>(gttBaseRem3 & 0xFFF00000));
        }
    }
}

uint64_t GpuXeHpCore::getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize, uint64_t stolenMemoryBaseAddress) const {
    const auto flatCcsSize = memoryBankSize / 256;
    const uint64_t flatCcsSizeAligned = alignUp(flatCcsSize, 20);
    return stolenMemoryBaseAddress + flatCcsSizeAligned;
}

PageTable *GpuXeHpCore::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    return new PML4(*this, physicalAddressAllocator, memoryBank);
}

void GpuXeHpCore::initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress) const {
    uint32_t mmioDevice = deviceIndex * mmioDeviceOffset;

    uint32_t mmioValue = static_cast<uint32_t>(flatCcsBaseAddress >> 8); // [8:31] base ptr
    mmioValue |= 1;                                                      // [0] enable bit
    stream.writeMMIO(mmioDevice + 0x4910, mmioValue);
}
} // namespace aub_stream
