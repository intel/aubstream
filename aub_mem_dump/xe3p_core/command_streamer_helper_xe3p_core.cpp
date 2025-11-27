/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"
#include "aub_mem_dump/page_table_pml5.h"
#include "aub_mem_dump/misc_helpers.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aubstream/product_family.h"
#include <algorithm>
#include <iostream>

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
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
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x00002058, 0x00000000), // CTX_WA_PTR_RCSUNIT
        MMIOPair(mmioEngine + 0x000020a8, 0x00000000), // IMR
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8284), // GFX_MODE

    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE

        // FORCE_TO_NONPRIV
        MMIOPair(mmioEngine + 0x000024d0, 0x00002920), // OAR_OAPERF_B0
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8284), // GFX_MODE
        MMIOPair(mmioEngine + 0x0000209C, 0x10000000), // MI_MODE
    };

    return engineMMIO;
}

template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperCccs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }
template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperRcs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }
template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperBcs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }
template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }
template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperVcs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }
template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperVecs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const { BaseClass::addFlushCommands(ringBuffer); }

template <>
void CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>::addFlushCommands(std::vector<uint32_t> &ringBuffer) const {
    // Pipe Control for flushes
    ringBuffer.push_back(0x7a001004); // QueueDrainMode

    ringBuffer.push_back(
        1 << 5 | // DCFlushEnable
        1 << 9   // IndirectStatePointersDisable
    );

    ringBuffer.push_back(0);
    ringBuffer.push_back(0);
    ringBuffer.push_back(0);
    ringBuffer.push_back(0);

    // Pipe Control for Invalidate
    ringBuffer.push_back(0x7a001004);
    ringBuffer.push_back(
        1 << 9 |  // IndirectStatePointersDisable
        1 << 2 |  // StateCacheInvalidationEnable
        1 << 3 |  // ConstantCacheInvalidationEnable
        1 << 5 |  // DCFlushEnable
        1 << 10 | // TextureCacheInvalidateEnable
        1 << 11 | // InstructionCacheInvalidateEnable
        1 << 29   // CommandCacheInvalidateEnable
    );
    ringBuffer.push_back(0);
    ringBuffer.push_back(0);
    ringBuffer.push_back(0);
    ringBuffer.push_back(0);
}

template <>
const MMIOList CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8284), // GFX_MODE
        MMIOPair(mmioEngine + 0x000020C4, 0xFFFF0306)  // CMD_CCTL_BCSUNIT BCS Read/Write point at MOCS#3
    };

    return engineMMIO;
}

GpuXe3pCore::GpuXe3pCore() {
    for (auto deviceId = 0u; deviceId < numSupportedDevices; deviceId++) {
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCCS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperCccs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_RCS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperRcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperBcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VCS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperVcs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_VECS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperVecs>>(deviceId);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>>(deviceId, 0);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS1] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>>(deviceId, 1);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS2] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>>(deviceId, 2);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_CCS3] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>>(deviceId, 3);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS1] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 1);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS2] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 2);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS3] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 3);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS4] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 4);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS5] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 5);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS6] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 6);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS7] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 7);
        commandStreamerHelperTable[deviceId][EngineType::ENGINE_BCS8] = std::make_unique<CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>>(deviceId, 8);
    }
}

PageTable *GpuXe3pCore::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    return new PML5(*this, physicalAddressAllocator, memoryBank);
}

uint32_t GpuXe3pCore::sizeToPAVPC(uint64_t wopcmMemorySize) const {
    static std::pair<uint32_t, uint64_t> sizeToPAVPCMappings[] = {
        {0x0, 1 * MB},
        {0x1, 2 * MB},
        {0x2, 4 * MB},
        {0x3, 8 * MB},
        {0x5, 16 * MB},
        {0x6, 32 * MB}};

    auto it = std::find_if(std::begin(sizeToPAVPCMappings), std::end(sizeToPAVPCMappings),
                           [wopcmMemorySize](const std::pair<std::uint32_t, uint64_t> &value) { return value.second == wopcmMemorySize; });
    if (it == std::end(sizeToPAVPCMappings)) {
        return 0xff;
    }
    return it->first;
}

uint32_t GpuXe3pCore::sizeToGMS(uint64_t dataStolenMemorySize) const {
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

bool GpuXe3pCore::isValidDataStolenMemorySize(uint64_t dataStolenMemorySize) const {
    return sizeToGMS(dataStolenMemorySize) != 0xff;
}

void GpuXe3pCore::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize) const {
    if (stream.getStreamMode() != aub_stream::mode::tbxShm3 && stream.getStreamMode() != aub_stream::mode::null) {
        auto flatCcsSize = memoryBankSize / 512;
        for (uint32_t i = 0; i < devicesCount; i++) {
            bool isLocalMemSupported = isMemorySupported(MEMORY_BANK_0 << i, 0x10000);

            uint64_t dsm = getDSMBaseAddress(i);

            // GSM is programmed by separate function, now it is enough to program DSM that is after Flat CCS and GSM
            stream.writeMMIO(i * mmioDeviceOffset + 0x1080c0, static_cast<uint32_t>(dsm));
            stream.writeMMIO(i * mmioDeviceOffset + 0x1080c4, static_cast<uint32_t>(dsm >> 32));

            // Set DSM and GSM sizes
            stream.writeMMIO(i * mmioDeviceOffset + 0x108040, 0x00000c0 | sizeToGMS(getDSMSize()) << 8);

            // WOPCM size
            stream.writeMMIO(i * mmioDeviceOffset + 0xC050, static_cast<uint32_t>(0x1f0000));

            // GuC WOPCM offset
            stream.writeMMIO(i * mmioDeviceOffset + 0xC340, static_cast<uint32_t>(0x600002));

            uint64_t wopcmBase = memoryBankSize - getWOPCMSize();
            stream.writeMMIO(i * mmioDeviceOffset + 0x1082C0, static_cast<uint32_t>(wopcmBase | 0x5 | (sizeToPAVPC(getWOPCMSize()) << 7)));
            stream.writeMMIO(i * mmioDeviceOffset + 0x1082C4, static_cast<uint32_t>(wopcmBase >> 32));

            // RC6CTXBASE
            uint64_t rc6Base = memoryBankSize - 64 * 1024;
            stream.writeMMIO(i * mmioDeviceOffset + 0xd48, static_cast<uint32_t>(rc6Base | 1));
            stream.writeMMIO(i * mmioDeviceOffset + 0xd4c, static_cast<uint32_t>(rc6Base >> 32));

            // RC6LOCATION
            stream.writeMMIO(i * mmioDeviceOffset + 0xd40, 0x80000001);

            // SG_ADDR_RANGE_TILE0
            stream.writeMMIO(i * mmioDeviceOffset + 0x1083a0, static_cast<uint32_t>((8 << 8) | 1));

            if (stream.getStreamMode() != aub_stream::mode::aubFile) {
                // put Flat CCS at the beginning of stolen memory
                uint64_t flatCcsBaseAddr = stolenMemory->getBaseAddress(i);
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

void GpuXe3pCore::initializeTileRangeMmio(AubStream &stream, uint32_t deviceIndex, uint64_t lmemBaseAddress, uint64_t lmemSize) const {
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

CommandStreamerHelper &GpuXe3pCore::getCommandStreamerHelper(uint32_t device, EngineType engineType) const {
    auto &csh = commandStreamerHelperTable[device][engineType];
    csh->gpu = this;
    return *csh;
}

const std::vector<EngineType> GpuXe3pCore::getSupportedEngines() const {
    static constexpr std::array<EngineType, 17> engines = {{ENGINE_BCS, ENGINE_VCS, ENGINE_VECS,
                                                            ENGINE_CCS, ENGINE_CCS1, ENGINE_CCS2, ENGINE_CCS3, ENGINE_CCCS,
                                                            ENGINE_BCS1, ENGINE_BCS2, ENGINE_BCS3, ENGINE_BCS4, ENGINE_BCS5,
                                                            ENGINE_BCS6, ENGINE_BCS7, ENGINE_BCS8}};
    return std::vector<EngineType>(engines.begin(), engines.end());
}

const MMIOList GpuXe3pCore::getGlobalMMIO() const {
    const MMIOList globalMMIO = {
        MMIOPair(0x00004b80, 0xffff1001),     // GACB_PERF_CTRL_REG
        MMIOPair(0x00007000, 0xffff0000),     // CACHE_MODE_0
        MMIOPair(0x00007004, 0xffff0000),     // CACHE_MODE_1
        MMIOPair(0x00009008, 0x00000200),     // IDICR
        MMIOPair(0x0000900c, 0x00001b40),     // SNPCR
        MMIOPair(0x0000b120, 0x14000002),     // LTCDREG
        MMIOPair(0x0000b134, 0xa0000000),     // L3ALLOCREG
        MMIOPair(0x0000b234, 0xa0000000),     // L3ALLOCREG_CCS0
        MMIOPair(0x0000ce90, 0x00030003),     // GFX_MULT_CTXT_CTL
        MMIOPair(0x0000cf58, 0x80000000),     // LMEM_CFG for local memory
        MMIOPair(0x0000e194, 0xffff0002),     // CHICKEN_SAMPLER_2
        MMIOPair(0x00014800, 0x00010001),     // RCU_MODE
        MMIOPair(0x00014804, 0x0fff0000),     // CCS_MODE
        MMIOPair(0x0001a0d8, 0x00020000),     // CS_DEBUG_MODE2_CCSUNIT
        MMIOPair(0x00042080, 0x00000000),     // CHICKEN_MISC_1
        MMIOPair(0x0000e7c8, 0x00400000),     // LSC_CHICKEN_BIT_0 bits 0:31
        MMIOPair(0x0000e7c8 + 4, 0x00001000), // LSC_CHICKEN_BIT_0 bits 32:63
    };
    return globalMMIO;
}

uint64_t GpuXe3pCore::getPPGTTExtraEntryBits(const AllocationParams::AdditionalParams &allocationParams) const {
    auto bits = toBitValue(PpgttEntryBits::atomicEnableBit);

    if (allocationParams.uncached) {
        bits |= (allocationParams.compressionEnabled) ? patIndex12 : patIndex3;
    } else {
        bits |= (allocationParams.compressionEnabled) ? patIndex9 : patIndex0;
    }

    return bits;
}

void GpuXe3pCore::initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const {
    uint32_t mmioDevice = 0;

    for (uint32_t device = 0; device < devicesCount; device++) {
        const auto &globalMMIO = getGlobalMMIO();
        for (const auto &mmioPair : globalMMIO) {
            stream.writeMMIO(mmioDevice + mmioPair.first, mmioPair.second);
        }

        const auto &globalMMIOPlatformSpecific = getGlobalMMIOPlatformSpecific();
        for (const auto &mmioPair : globalMMIOPlatformSpecific) {
            stream.writeMMIO(mmioDevice + mmioPair.first, mmioPair.second);
        }

        mmioDevice += mmioDeviceOffset;
    }
}

} // namespace aub_stream
