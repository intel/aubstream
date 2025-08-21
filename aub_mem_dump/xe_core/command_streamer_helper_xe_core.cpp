/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe_core/command_streamer_helper_xe_core.h"
#include "aub_mem_dump/align_helpers.h"

namespace aub_stream {

template <>
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperRcs>::getEngineMMIO() const {
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
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {
        MMIOPair(mmioEngine + 0x0000229c, 0xffff8280), // GFX_MODE
    };

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperVcs>::getEngineMMIO() const {
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
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperVecs>::getEngineMMIO() const {
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
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperCcs>::getEngineMMIO() const {
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
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperCccs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false); // not supported on XE

    return engineMMIO;
}

template <>
const MMIOList CommandStreamerHelperXeCore<CommandStreamerHelperLinkBcs>::getEngineMMIO() const {
    const MMIOList engineMMIO = {};
    assert(false); // not supported on XE

    return engineMMIO;
}

void GpuXeCore::initializeDefaultMemoryPools(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {
    if (IsAnyTbxMode(stream.getStreamMode())) {
        for (uint32_t i = 0; i < devicesCount; i++) {
            // put flat ccs at the beginning of stolen memory

            uint64_t flatCcsBaseAddr = stolenMemory.getBaseAddress(i);
            initializeFlatCcsBaseAddressMmio(stream, i, flatCcsBaseAddr);
        }
    }
}

void GpuXeCore::setMemoryBankSize(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const {
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

void GpuXeCore::setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize, const StolenMemory &stolenMemory) const {
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

uint64_t GpuXeCore::getGGTTBaseAddress(uint32_t device, uint64_t memoryBankSize, uint64_t stolenMemoryBaseAddress) const {
    const auto flatCcsSize = memoryBankSize / 256;
    const uint64_t flatCcsSizeAligned = alignUp(flatCcsSize, 20);
    return stolenMemoryBaseAddress + flatCcsSizeAligned;
}

PageTable *GpuXeCore::allocatePPGTT(PhysicalAddressAllocator *physicalAddressAllocator, uint32_t memoryBank, uint64_t gpuAddressSpace) const {
    return new PML4(*this, physicalAddressAllocator, memoryBank);
}

void GpuXeCore::initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress) const {
    uint32_t mmioDevice = deviceIndex * mmioDeviceOffset;

    uint32_t mmioValue = static_cast<uint32_t>(flatCcsBaseAddress >> 8); // [8:31] base ptr
    mmioValue |= 1;                                                      // [0] enable bit
    stream.writeMMIO(mmioDevice + 0x4910, mmioValue);
}

bool GpuXeCore::isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const {
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

} // namespace aub_stream
