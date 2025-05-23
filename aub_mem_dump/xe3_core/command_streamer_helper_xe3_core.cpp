/*
 * Copyright (C) 2024-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3_core/command_streamer_helper_xe3_core.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aub_mem_dump/misc_helpers.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aubstream/product_family.h"
#include <algorithm>
#include <iostream>

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperRcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00007304), // COMMON_SLICE_CHICKEN3
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE

    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00002920), // OAR_OAPERF_B0
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3Core<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

GpuXe3Core::GpuXe3Core() {
    for (auto deviceId = 0u; deviceId < GpuXeHpCore::numSupportedDevices; deviceId++) {
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCCS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperCccs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperRcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperBcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperVcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperVecs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperCcs>>(deviceId, 0);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS8] = std::make_unique<CommandStreamerHelperXe3Core<CommandStreamerHelperLinkBcs>>(deviceId, 8);
    }
}

PageTable *GpuXe3Core::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    return new PML5(*this, physicalAddressAllocator, memoryBank);
}

uint32_t GpuXe3Core::sizeToGMS(uint64_t dataStolenMemorySize) const {
    static std::pair<uint32_t, uint64_t> sizeToGMSMappings[] = {
        {0x00, 0 * MB},
        {0x01, 32 * MB},
        {0x02, 64 * MB},
        {0x03, 96 * MB},
        {0x04, 128 * MB},
        {0xf0, 4 * MB},
        {0xf1, 8 * MB},
        {0xf2, 12 * MB},
        {0xf3, 16 * MB},
        {0xf4, 20 * MB},
        {0xf5, 24 * MB},
        {0xf6, 28 * MB},
        {0xf7, 32 * MB},
        {0xf8, 36 * MB},
        {0xf9, 40 * MB},
        {0xfa, 44 * MB},
        {0xfb, 48 * MB},
        {0xfc, 52 * MB},
        {0xfd, 56 * MB},
        {0xfe, 60 * MB}};

    auto it = std::find_if(std::begin(sizeToGMSMappings), std::end(sizeToGMSMappings),
                           [dataStolenMemorySize](const std::pair<std::uint32_t, uint64_t> &value) { return value.second == dataStolenMemorySize; });
    if (it == std::end(sizeToGMSMappings)) {
        return 0xff;
    }
    return it->first;
}

bool GpuXe3Core::isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const {
    return sizeToGMS(dataStolenMemorySize) != 0xff;
}

void GpuXe3Core::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {
    if ((stream.getStreamMode() == aub_stream::mode::tbx || stream.getStreamMode() == aub_stream::mode::aubFileAndTbx || stream.getStreamMode() == aub_stream::mode::tbxShm ||
         stream.getStreamMode() == aub_stream::mode::tbxShm4 || stream.getStreamMode() == aub_stream::mode::aubFileAndShm || stream.getStreamMode() == aub_stream::mode::aubFileAndShm4)) {
        auto flatCcsSize = memoryBankSize / 512;
        for (uint32_t i = 0; i < devicesCount; i++) {
            bool isLocalMemSupported = isMemorySupported(MEMORY_BANK_0 << i, 0x10000);

            uint64_t gsm = getGGTTBaseAddress(i, memoryBankSize, stolenMemory.getBaseAddress(i));
            uint64_t dsm = gsm + StolenMemory::ggttSize;

            // GSM is programmed by separate function, now it is enough to program DSM that is after Flat CCS and GSM
            stream.writeMMIO(i * mmioDeviceOffset + 0x1080c0, static_cast<uint32_t>(dsm));
            stream.writeMMIO(i * mmioDeviceOffset + 0x1080c4, static_cast<uint32_t>(dsm >> 32));

            // Set DSM and GSM sizes
            stream.writeMMIO(i * mmioDeviceOffset + 0x108040, 0x00000c0 | sizeToGMS(stolenMemory.dsmSize) << 8);

            // WOPCM size
            stream.writeMMIO(i * mmioDeviceOffset + 0xC050, static_cast<uint32_t>(0x1f0000));

            // GuC WOPCM offset
            stream.writeMMIO(i * mmioDeviceOffset + 0xC340, static_cast<uint32_t>(0x600002));

            uint64_t wopcmBase = memoryBankSize - 8 * 1024 * 1024;
            stream.writeMMIO(i * mmioDeviceOffset + 0x1082C0, static_cast<uint32_t>(wopcmBase | 0x5 | 0x180));
            stream.writeMMIO(i * mmioDeviceOffset + 0x1082C4, static_cast<uint32_t>(wopcmBase >> 32));

            // RC6CTXBASE
            uint64_t rc6Base = memoryBankSize - 64 * 1024;
            stream.writeMMIO(i * mmioDeviceOffset + 0xd48, static_cast<uint32_t>(rc6Base | 1));
            stream.writeMMIO(i * mmioDeviceOffset + 0xd4c, static_cast<uint32_t>(rc6Base >> 32));

            // RC6LOCATION
            stream.writeMMIO(i * mmioDeviceOffset + 0xd40, 0x80000001);

            // SG_ADDR_RANGE_TILE0
            stream.writeMMIO(i * mmioDeviceOffset + 0x1083a0, static_cast<uint32_t>((8 << 8) | 1));

            // put Flat CCS at the beginning of stolen memory
            uint64_t flatCcsBaseAddr = stolenMemory.getBaseAddress(i);
            if (isLocalMemSupported) {
                uint32_t val = stream.readMMIO(0x9118);
                uint64_t numL3Banks = countBits(val >> 16);
                if (numL3Banks == 0) {
                    std::cerr << "The number of L3 banks cannot be zero" << std::endl;
                    return;
                }
                flatCcsSize /= numL3Banks;
                flatCcsBaseAddr /= numL3Banks;
            }
            initializeFlatCcsBaseAddressMmio(stream, i, flatCcsBaseAddr, flatCcsSize);
        }
    } else if (stream.getStreamMode() == aub_stream::mode::tbxShm3) {
        for (uint32_t i = 0; i < devicesCount; i++) {
            // SHMv3 does not support neither tile range checks nor flat ccs
            initializeFlatCcsBaseAddressMmio(stream, i, 0, 0);
            if (isMemorySupported(MEMORY_BANK_0 << i, 0x10000)) {
                initializeTileRangeMmio(stream, i, 0, 0);
            }
        }
    }
}

void GpuXe3Core::initializeTileRangeMmio(AubStream &stream, uint32_t deviceIndex, uint64_t lmemBaseAddress, uint64_t lmemSize) const {
    uint32_t mmioDevice = deviceIndex * mmioDeviceOffset;
    assert(lmemSize % GB == 0);
    assert(lmemBaseAddress % GB == 0);
    uint64_t lmemSizeInGB = lmemSize / GB;
    uint64_t lmemBaseAddressInGB = lmemBaseAddress / GB;
    assert(lmemSizeInGB <= 127);
    uint32_t mmioVal = static_cast<uint32_t>(lmemSizeInGB) << 8 | static_cast<uint32_t>(lmemBaseAddressInGB) << 1;
    if (lmemSizeInGB > 0) {
        mmioVal |= 1; // [0] - enable tile range
    }
    stream.writeMMIO(mmioDevice + 0x4900, mmioVal);
    stream.writeMMIO(mmioDevice + 0x384900, mmioVal);
}

void GpuXe3Core::initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress, uint64_t size) const {
    uint32_t mmioDevice = deviceIndex * mmioDeviceOffset;

    uint32_t flatCcsSize = static_cast<uint32_t>((size / (128 * KB)));
    flatCcsSize = std::min(flatCcsSize, 0x3FFu);

    // address low
    {
        assert((flatCcsBaseAddress & 0x3F) == 0); // [0:5] - must be 0

        uint32_t mmioLow = static_cast<uint32_t>(flatCcsBaseAddress & 0xFFFFFFFF); // [6:31] - low address
        if (flatCcsSize > 0) {
            mmioLow |= 1; // [0] - enable flat ccs
        }
        stream.writeMMIO(mmioDevice + 0x8800, mmioLow);
        stream.writeMMIO(mmioDevice + 0x1344b0, mmioLow);
    }

    // address high
    {
        uint32_t addressHigh = static_cast<uint32_t>(flatCcsBaseAddress >> 32);
        assert((addressHigh & 0xFFFFF00) == 0); // [8:31] - must be 0

        uint32_t mmioHigh = addressHigh; // [0:7] - high address
        mmioHigh |= (flatCcsSize << 14); // [14:23] - flat ccs size

        stream.writeMMIO(mmioDevice + 0x8804, mmioHigh);
        stream.writeMMIO(mmioDevice + 0x1344b4, mmioHigh);
    }

    {
        uint32_t mmioVal = 0;
        if (flatCcsSize > 0) {
            mmioVal |= 1; // [0] - enable flat ccs
        }
        stream.writeMMIO(mmioDevice + 0x4910, mmioVal);
        stream.writeMMIO(mmioDevice + 0x384910, mmioVal);
    }
}
const std::vector<EngineType> GpuXe3Core::getSupportedEngines() const {
    static constexpr std::array<EngineType, 6> engines = {{ENGINE_BCS, ENGINE_VCS, ENGINE_VECS,
                                                           ENGINE_CCS, ENGINE_CCCS, ENGINE_BCS8}};
    return std::vector<EngineType>(engines.begin(), engines.end());
}

constexpr uint32_t GpuXe3Core::getPatIndexMmioAddr(uint32_t index) {
    assert(index <= 31);
    uint32_t address = 0x4800 + (index * 4);

    if (index >= 8) {
        address += 0x28; // gap between 7 and 8
    }

    return address;
}

const MMIOList GpuXe3Core::getGlobalMMIO() const {
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

        // bits: 10: NO_PROMOTE, 9: COMP_EN, 7-6: L3_CLOS, 5-4: L3_CACHE_POLICY, 3-2:L4_CACHE_POLICY, 1-0: COH_MODE
        MMIOPair(getPatIndexMmioAddr(0), 0b0000'0000'1100),  // L3
        MMIOPair(getPatIndexMmioAddr(1), 0b0000'0000'1110),  // L3 (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(2), 0b0000'0000'1111),  // L3 (2-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(3), 0b0000'0011'1100),  // UC
        MMIOPair(getPatIndexMmioAddr(4), 0b0000'0011'0010),  // L4 (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(5), 0b0000'0011'1110),  // UC (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(6), 0b0100'0001'1100),  // L3:XD
        MMIOPair(getPatIndexMmioAddr(7), 0b0000'0011'0011),  // L4 (2-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(8), 0b0000'0011'0000),  // L4
        MMIOPair(getPatIndexMmioAddr(9), 0b0010'0000'1100),  // L3 Compressed
        MMIOPair(getPatIndexMmioAddr(10), 0b0010'0011'0000), // L4 Compressed
        MMIOPair(getPatIndexMmioAddr(11), 0b0110'0001'1100), // L3:XD Compressed
        MMIOPair(getPatIndexMmioAddr(12), 0b0010'0011'1100), // UC Compressed
        MMIOPair(getPatIndexMmioAddr(13), 0b0000'0000'0000), // L3 + L4
        MMIOPair(getPatIndexMmioAddr(14), 0b0010'0000'0000), // L3 + L4 Commpressed
        MMIOPair(getPatIndexMmioAddr(15), 0b0110'0001'0100), // L3:XD + L4:WT Compressed
        // PatIndex 16..19 Reserved
        MMIOPair(getPatIndexMmioAddr(20), 0b0000'0100'1100), // CLOS1 L3
        MMIOPair(getPatIndexMmioAddr(21), 0b0010'0100'1100), // CLOS1 L3 Compressed
        MMIOPair(getPatIndexMmioAddr(22), 0b0000'0100'1110), // CLOS1 L3 (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(23), 0b0000'0100'1111), // CLOS1 L3 (2-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(24), 0b0000'1000'1100), // CLOS2 L3
        MMIOPair(getPatIndexMmioAddr(25), 0b0010'1000'1100), // CLOS2 L3 Compressed
        MMIOPair(getPatIndexMmioAddr(26), 0b0000'1000'1110), // CLOS2 L3 (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(27), 0b0000'1000'1111), // CLOS2 L3 (2-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(28), 0b0000'1100'1100), // CLOS3 L3
        MMIOPair(getPatIndexMmioAddr(29), 0b0010'1100'1100), // CLOS3 L3 Compressed
        MMIOPair(getPatIndexMmioAddr(30), 0b0000'1100'1110), // CLOS3 L3 (1-Way Coherent)
        MMIOPair(getPatIndexMmioAddr(31), 0b0000'1100'1111), // CLOS3 L3 (2-Way Coherent)
    };
    return globalMMIO;
}

uint64_t GpuXe3Core::getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const {
    auto bits = toBitValue(PpgttEntryBits::atomicEnableBit);

    if (allocationParams.uncached) {
        bits |= (allocationParams.compressionEnabled) ? patIndex12 : patIndex2;
    } else {
        bits |= (allocationParams.compressionEnabled) ? patIndex9 : patIndex0;
    }

    return bits;
}

} // namespace aub_stream
