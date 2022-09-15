/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "aub_mem_dump/tbx_shm_stream.h"
#include "aub_mem_dump/tbx_stream.h"
#include "headers/aubstream.h"
#include "headers/engine_node.h"
#include "headers/physical_allocation_info.h"
#include "headers/shared_mem_info.h"
#include <cassert>
#include <exception>
#include <stdexcept>

namespace aub_stream {

extern CommandStreamerHelper &getCommandStreamerHelper(uint32_t gfxFamily, uint32_t device, EngineType engine);

AubManagerImp::AubManagerImp(const Gpu &gpu, const struct AubManagerOptions &options)
    : gpu(gpu), devicesCount(options.devicesCount), localMemorySupported(options.localMemorySupported), stepping(options.stepping),
      memoryBankSize(options.memoryBankSize), streamMode(options.mode), gpuAddressSpace(options.gpuAddressSpace), sharedMemoryInfo(options.sharedMemoryInfo),
      enableThrow(options.throwOnError) {
    initialize();
}

AubManagerImp::~AubManagerImp() = default;

void AubManagerImp::initialize() {
    bool createAubFileStream = streamMode == aub_stream::mode::aubFile || streamMode == aub_stream::mode::aubFileAndTbx;
    bool createTbxStream = streamMode == aub_stream::mode::tbx || streamMode == aub_stream::mode::aubFileAndTbx;
    bool createTbxShmStream = streamMode == aub_stream::mode::tbxShm;

    if (createAubFileStream) {
        streamAub = std::make_unique<AubFileStream>();
    }

    if (createTbxStream) {
        streamTbx = std::make_unique<TbxStream>();
        streamTbx->init(stepping, gpu.deviceId, gpu.gfxCoreFamily);

        gpu.initializeGlobalMMIO(*streamTbx, devicesCount, memoryBankSize, stepping);
        gpu.setMemoryBankSize(*streamTbx, devicesCount, memoryBankSize);
        gpu.setGGTTBaseAddresses(*streamTbx, devicesCount, memoryBankSize);
    } else if (createTbxShmStream) {
        streamTbxShm = std::make_unique<TbxShmStream>();
        assert(sharedMemoryInfo.sysMemBase != nullptr);
        if ((sharedMemoryInfo.sysMemBase == nullptr) && enableThrow) {
            throw std::logic_error("Trying to use shared memory but no shared memory for system memory given.");
        }
        streamTbxShm->init(stepping, gpu.deviceId, gpu.gfxCoreFamily, &sharedMemoryInfo);
        streamTbxShm->enableThrowOnError(enableThrow);

        gpu.initializeGlobalMMIO(*streamTbxShm, devicesCount, memoryBankSize, stepping);
        gpu.setMemoryBankSize(*streamTbxShm, devicesCount, memoryBankSize);
        gpu.setGGTTBaseAddresses(*streamTbxShm, devicesCount, memoryBankSize);
    }

    if (streamMode == aub_stream::mode::aubFileAndTbx) {
        streamAubTbx = std::make_unique<AubTbxStream>(*streamAub, *streamTbx);
    }

    physicalAddressAllocator = std::make_unique<PhysicalAddressAllocator>(devicesCount, memoryBankSize, localMemorySupported);

    ppgtts.resize(devicesCount);
    ggtts.resize(devicesCount);

    for (uint32_t i = 0; i < devicesCount; i++) {
        uint32_t memoryBank = MemoryBank::MEMORY_BANK_SYSTEM;
        uint64_t gttBaseAddress = 0; // Default to 0 base address of GGTT for non Local Memory
        if (localMemorySupported || gpu.requireLocalMemoryForPageTables()) {
            memoryBank = MemoryBank::MEMORY_BANK_0 << i;
            gttBaseAddress = gpu.getGGTTBaseAddress(i, memoryBankSize);
        }

        ppgtts[i] = std::unique_ptr<PageTable>(gpu.allocatePPGTT(physicalAddressAllocator.get(), memoryBank, gpuAddressSpace));
        ggtts[i] = std::unique_ptr<GGTT>(gpu.allocateGGTT(physicalAddressAllocator.get(), memoryBank, gttBaseAddress));
    }
    gpu.initializeDefaultMemoryPools(*getStream(), devicesCount, memoryBankSize);
}

void AubManagerImp::open(const std::string &aubFileName) {
    if (streamMode == aub_stream::mode::aubFile || streamMode == aub_stream::mode::aubFileAndTbx) {
        streamAub->open(aubFileName.c_str());
        streamAub->init(stepping, gpu.deviceId, gpu.gfxCoreFamily);
        gpu.initializeGlobalMMIO(*streamAub, devicesCount, memoryBankSize, stepping);
        gpu.setMemoryBankSize(*streamAub, devicesCount, memoryBankSize);
    }

    for (auto &ctxt : hwContexts) {
        ctxt->initialize();
    }
}

void AubManagerImp::close() {
    for (auto &ctxt : hwContexts) {
        ctxt->release();
    }

    if (streamAub) {
        streamAub->close();
    }
}

bool AubManagerImp::isOpen() {
    if (streamAub) {
        return streamAub->isOpen();
    }
    return false;
}

const std::string AubManagerImp::getFileName() {
    if (streamAub) {
        return streamAub->getFileName();
    }
    return {};
}

void AubManagerImp::pause(bool onoff) {
    if (streamAubTbx) {
        streamAubTbx->pauseAubFileStream(onoff);
    }
}

void AubManagerImp::addComment(const char *message) {
    AubStream *stream = getStream();
    if (stream) {
        stream->addComment(message);
    }
}

HardwareContext *AubManagerImp::createHardwareContext(uint32_t device, uint32_t engine, uint32_t flags) {
    auto &csTraits = gpu.getCommandStreamerHelper(device, static_cast<EngineType>(engine));
    AubStream *stream = getStream();

    auto ctxt = new HardwareContextImp(device, *stream, csTraits, *ggtts[device], *ppgtts[device], flags);

    hwContexts.push_back(ctxt);

    return ctxt;
}

void AubManagerImp::adjustPageSize(uint32_t memoryBanks, size_t &pageSize) {
    auto &csTraits = gpu.getCommandStreamerHelper(0, static_cast<EngineType>(ENGINE_RCS));

    if (!csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize))) {
        pageSize = pageSize == 65536u ? 4096u : 65536u;
    }
    // make sure this combination is still valid
    assert(csTraits.isMemorySupported(memoryBanks, static_cast<uint32_t>(pageSize)));
}

void AubManagerImp::writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks,
                                int hint, size_t pageSize) {
    // fallback to new interface
    writeMemory2(AllocationParams(gfxAddress, memory, size, memoryBanks, hint, pageSize));
}

void AubManagerImp::writeMemory2(AllocationParams allocationParams) {
    adjustPageSize(allocationParams.memoryBanks, allocationParams.pageSize);
    AubStream *stream = getStream();

    auto pageTableEntries = stream->writeMemory(ppgtts[0].get(), allocationParams);

    for (uint32_t i = 1; i < ppgtts.size(); i++) {
        stream->cloneMemory(ppgtts[i].get(), pageTableEntries, allocationParams);
    }
}

void AubManagerImp::writePageTableEntries(uint64_t gfxAddress, size_t size, uint32_t memoryBanks,
                                          int hint, std::vector<PageInfo> &lastLevelPages, size_t pageSize) {

    adjustPageSize(memoryBanks, pageSize);
    AubStream *stream = getStream();

    auto pageTableEntries = stream->writeMemory(ppgtts[0].get(), AllocationParams(gfxAddress, nullptr, size, memoryBanks, hint, pageSize));

    for (uint32_t i = 1; i < ppgtts.size(); i++) {
        stream->cloneMemory(ppgtts[i].get(), pageTableEntries, AllocationParams(gfxAddress, nullptr, size, memoryBanks, 0, pageSize));
    }

    lastLevelPages.insert(lastLevelPages.end(), pageTableEntries.begin(), pageTableEntries.end());
}

void AubManagerImp::writePhysicalMemoryPages(const void *memory, std::vector<PageInfo> &pages, size_t size, int hint) {

    assert(pages.size() >= 1);
    AubStream *stream = getStream();
    stream->writePhysicalMemoryPages(memory, size, pages, hint);
}

void AubManagerImp::freeMemory(uint64_t gfxAddress, size_t size) {
    AubStream *stream = getStream();

    for (auto &ppgtt : ppgtts) {
        stream->freeMemory(ppgtt.get(), gfxAddress, size);
    }
}

bool AubManagerImp::reservePhysicalMemory(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) {
    adjustPageSize(allocationParams.memoryBanks, allocationParams.pageSize);
    auto size = allocationParams.size;
    auto memoryBanks = allocationParams.memoryBanks;

    if (size == 0) {
        return false;
    }

    if (allocationParams.pageSize == 0) {
        return false;
    }

    if (memoryBanks != 0) {
        if (memoryBanks & (memoryBanks - 1)) {
            return false;
        }
    }

    auto pageSize = allocationParams.pageSize;
    size_t sizePageAligned = (size + pageSize - 1) & ~(pageSize - 1);

    physicalAllocInfo.memoryBank = memoryBanks;
    physicalAllocInfo.pageSize = pageSize;
    physicalAllocInfo.size = size;
    physicalAllocInfo.physicalAddress = physicalAddressAllocator->reservePhysicalMemory(allocationParams.memoryBanks, sizePageAligned, pageSize);

    return true;
}

bool AubManagerImp::mapGpuVa(uint64_t gfxAddress, size_t size, const PhysicalAllocationInfo physicalAllocInfo) {
    AubStream *stream = getStream();

    AllocationParams allocationParams(gfxAddress, nullptr, size, physicalAllocInfo.memoryBank, 0, physicalAllocInfo.pageSize);

    for (auto &ppgtt : ppgtts) {
        stream->mapGpuVa(ppgtt.get(), allocationParams, physicalAllocInfo.physicalAddress);
    }

    return true;
}

AubStream *AubManagerImp::getStream() {
    AubStream *stream = nullptr;

    if (streamMode == mode::aubFile) {
        stream = streamAub.get();
    } else if (streamMode == mode::tbx) {
        stream = streamTbx.get();
    } else if (streamMode == mode::aubFileAndTbx) {
        stream = streamAubTbx.get();
    } else if (streamMode == mode::tbxShm) {
        stream = streamTbxShm.get();
    } else {
        assert(false);
    }

    return stream;
}

AubManager *AubManager::create(uint32_t productFamily, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t mode, uint64_t gpuAddressSpace) {
    auto gpu = getGpu(static_cast<PRODUCT_FAMILY>(productFamily));
    if (nullptr != gpu) {
        AubManagerOptions internal_options;
        internal_options.productFamily = productFamily;
        internal_options.devicesCount = devicesCount;
        internal_options.memoryBankSize = memoryBankSize;
        internal_options.stepping = stepping;
        internal_options.localMemorySupported = localMemorySupported;
        internal_options.mode = mode;
        internal_options.gpuAddressSpace = gpuAddressSpace;
        return new AubManagerImp(*gpu, internal_options);
    }
    return nullptr;
}

AubManager *AubManager::create(uint32_t productFamily, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping, bool localMemorySupported, uint32_t mode, uint64_t gpuAddressSpace, SharedMemoryInfo sharedMemoryInfo) {
    auto gpu = getGpu(static_cast<PRODUCT_FAMILY>(productFamily));
    if (nullptr != gpu) {
        AubManagerOptions internal_options;
        internal_options.productFamily = productFamily;
        internal_options.devicesCount = devicesCount;
        internal_options.memoryBankSize = memoryBankSize;
        internal_options.stepping = stepping;
        internal_options.localMemorySupported = localMemorySupported;
        internal_options.mode = mode;
        internal_options.gpuAddressSpace = gpuAddressSpace;
        internal_options.sharedMemoryInfo = sharedMemoryInfo;
        return new AubManagerImp(*gpu, internal_options);
    }
    return nullptr;
}

AubManager *AubManager::create(const struct AubManagerOptions &options) {
    auto gpu = getGpu(static_cast<PRODUCT_FAMILY>(options.productFamily));
    if (nullptr != gpu) {
        return new AubManagerImp(*gpu, options);
    }
    return nullptr;
}

} // namespace aub_stream
