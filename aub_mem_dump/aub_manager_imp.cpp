/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_manager_imp.h"
#include "aub_mem_dump/aub_file_stream.h"
#include "aub_mem_dump/aub_tbx_stream.h"
#include "aub_mem_dump/aub_shm_stream.h"
#include "aub_mem_dump/command_streamer_helper.h"
#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/gpu.h"
#include "aub_mem_dump/hardware_context_imp.h"
#include "aub_mem_dump/null_hardware_context.h"
#include "aub_mem_dump/memory_banks.h"
#include "aub_mem_dump/page_table.h"
#include "aub_mem_dump/physical_address_allocator.h"
#include "aub_mem_dump/tbx_shm_stream.h"
#include "aub_mem_dump/tbx_stream.h"
#include "aub_mem_dump/settings.h"
#include "aub_mem_dump/misc_helpers.h"

#include "aubstream/aubstream.h"
#include "aubstream/engine_node.h"
#include "aubstream/physical_allocation_info.h"
#include "aubstream/shared_mem_info.h"

#include <cassert>
#include <exception>
#include <stdexcept>

namespace aub_stream {
Settings *globalSettings = nullptr;

AubManagerImp::AubManagerImp(std::unique_ptr<Gpu> gpu, const struct AubManagerOptions &options) : gpu(std::move(gpu)),
                                                                                                  devicesCount(options.devicesCount),
                                                                                                  memoryBankSize(options.memoryBankSize),
                                                                                                  localMemorySupported(options.localMemorySupported),
                                                                                                  streamMode(options.mode),
                                                                                                  stepping(options.stepping),
                                                                                                  gpuAddressSpace(options.gpuAddressSpace),
                                                                                                  stolenMem(StolenMemory::CreateStolenMemory(options.mode == aub_stream::mode::tbxShm3, options.devicesCount, options.memoryBankSize, options.dataStolenMemorySize ? options.dataStolenMemorySize : this->gpu->getDefaultDataStolenMemorySize())),
                                                                                                  sharedMemoryInfo(options.sharedMemoryInfo),
                                                                                                  enableThrow(options.throwOnError) {
    groupContextHelper = std::make_unique<GroupContextHelper>();
    auto contextGroupCount = this->gpu->getContextGroupCount();

    for (size_t i = 0; i < arrayCount(groupContextHelper->contextGroups); i++) {
        for (size_t j = 0; j < arrayCount(groupContextHelper->contextGroups[i]); j++) {
            groupContextHelper->contextGroups[i][j].reserve(4);
            groupContextHelper->contextGroups[i][j].resize(1);
            groupContextHelper->contextGroups[i][j][0].contexts.resize(contextGroupCount);
            groupContextHelper->groupIds[i][j] = 0;
        }
    }
}

AubManagerImp::~AubManagerImp() {
    if (globalSettings == settings.get()) {
        globalSettings = nullptr;
    }
}

void AubManagerImp::initialize() {
    if (!stolenMem || !gpu->isValidDataStolenMemorySize(stolenMem->dsmSize)) {
        return;
    }

    createStream();

    if (!getStream()) {
        return;
    }

    if (streamMode == aub_stream::mode::tbxShm3) {
        physicalAddressAllocator = std::make_unique<PhysicalAddressAllocatorHeap>();
    } else if (streamMode == aub_stream::mode::tbxShm4 || streamMode == aub_stream::mode::aubFileAndShm4) {
        physicalAddressAllocator = std::make_unique<PhysicalAddressAllocatorSimpleAndSHM4Mapper>(devicesCount, memoryBankSize, localMemorySupported, &sharedMemoryInfo);
    } else {
        physicalAddressAllocator = std::make_unique<PhysicalAddressAllocatorSimple>(devicesCount, memoryBankSize, localMemorySupported);
    }

    if (streamMode == aub_stream::mode::aubFileAndTbx || streamMode == aub_stream::mode::tbx) {
        streamTbx->init(stepping, *gpu);
        gpu->initializeGlobalMMIO(*streamTbx, devicesCount, memoryBankSize, stepping);
        gpu->setMemoryBankSize(*streamTbx, devicesCount, memoryBankSize);
        gpu->setGGTTBaseAddresses(*streamTbx, devicesCount, memoryBankSize, *stolenMem);
    } else if (streamMode == aub_stream::mode::tbxShm || streamMode == aub_stream::mode::aubFileAndShm) {
        streamTbxShm->init([this](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
            uint64_t memSize = isLocalMemory ? sharedMemoryInfo.localMemSize : sharedMemoryInfo.sysMemSize;
            availableSize = static_cast<size_t>(memSize - physAddress);
            p = (isLocalMemory ? sharedMemoryInfo.localMemBase : sharedMemoryInfo.sysMemBase) + physAddress; });
        streamTbxShm->enableThrowOnError(enableThrow);
        gpu->initializeGlobalMMIO(*streamTbxShm, devicesCount, memoryBankSize, stepping);
        gpu->setMemoryBankSize(*streamTbxShm, devicesCount, memoryBankSize);
        gpu->setGGTTBaseAddresses(*streamTbxShm, devicesCount, memoryBankSize, *stolenMem);
    } else if (streamMode == aub_stream::mode::tbxShm4 || streamMode == aub_stream::mode::aubFileAndShm4) {
        streamTbxShm->init([this](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
            static_cast<PhysicalAddressAllocatorSimpleAndSHM4Mapper *>(physicalAddressAllocator.get())->translatePhysicalAddressToSystemMemory(physAddress, size, isLocalMemory, p, availableSize);
        });
        streamTbxShm->enableThrowOnError(enableThrow);
        gpu->initializeGlobalMMIO(*streamTbxShm, devicesCount, memoryBankSize, stepping);
        gpu->setMemoryBankSize(*streamTbxShm, devicesCount, memoryBankSize);
        gpu->setGGTTBaseAddresses(*streamTbxShm, devicesCount, memoryBankSize, *stolenMem);
    } else if (streamMode == aub_stream::mode::tbxShm3) {
        streamTbxShm->init([](uint64_t physAddress, size_t size, bool isLocalMemory, void *&p, size_t &availableSize) {
            availableSize = size;
            p = reinterpret_cast<void *>(physAddress); });
        streamTbxShm->enableThrowOnError(enableThrow);
        gpu->initializeGlobalMMIO(*streamTbxShm, devicesCount, memoryBankSize, stepping);
        gpu->setMemoryBankSize(*streamTbxShm, devicesCount, memoryBankSize);
        gpu->setGGTTBaseAddresses(*streamTbxShm, devicesCount, memoryBankSize, *stolenMem);
    }

    ppgtts.resize(devicesCount);
    ggtts.resize(devicesCount);

    for (uint32_t i = 0; i < devicesCount; i++) {
        uint64_t stolenBaseAddress = stolenMem->getBaseAddress(i);
        uint32_t memoryBank = MemoryBank::MEMORY_BANK_SYSTEM;
        uint64_t gttBaseAddress = gpu->getGGTTBaseAddress(i, memoryBankSize, stolenBaseAddress);

        if (localMemorySupported || gpu->requireLocalMemoryForPageTables()) {
            memoryBank = MemoryBank::MEMORY_BANK_0 << i;
        }
        if (streamMode == aub_stream::mode::tbxShm4 || streamMode == aub_stream::mode::aubFileAndShm4) {
            static_cast<PhysicalAddressAllocatorSimpleAndSHM4Mapper *>(physicalAddressAllocator.get())->mapSystemMemoryToPhysicalAddress(stolenBaseAddress, static_cast<size_t>(memoryBankSize - stolenBaseAddress), 0x10000, memoryBank != MemoryBank::MEMORY_BANK_SYSTEM, nullptr);
        }
        ppgtts[i] = std::unique_ptr<PageTable>(gpu->allocatePPGTT(physicalAddressAllocator.get(), memoryBank, gpuAddressSpace));
        ggtts[i] = std::unique_ptr<GGTT>(gpu->allocateGGTT(physicalAddressAllocator.get(), memoryBank, gttBaseAddress));
    }

    gpu->initializeDefaultMemoryPools(*getStream(), devicesCount, memoryBankSize, *stolenMem);
    if (streamTbx) {
        gpu->injectMMIOs(*streamTbx, devicesCount);
    } else if (streamTbxShm) {
        gpu->injectMMIOs(*streamTbxShm, devicesCount);
    }
}

void AubManagerImp::createStream() {
    bool createAubFileStream = streamMode == aub_stream::mode::aubFile || streamMode == aub_stream::mode::aubFileAndTbx ||
                               streamMode == aub_stream::mode::aubFileAndShm || streamMode == aub_stream::mode::aubFileAndShm4;
    bool createTbxStream = streamMode == aub_stream::mode::tbx || streamMode == aub_stream::mode::aubFileAndTbx;
    bool createTbxShmStream = streamMode == aub_stream::mode::tbxShm || streamMode == aub_stream::mode::aubFileAndShm;
    bool createTbxShm3Stream = streamMode == aub_stream::mode::tbxShm3;
    bool createTbxShm4Stream = streamMode == aub_stream::mode::tbxShm4 || streamMode == aub_stream::mode::aubFileAndShm4;

    if (createAubFileStream) {
        streamAub = std::make_unique<AubFileStream>();
        streamAub->enableThrowOnError(enableThrow);
    }
    if (createTbxStream) {
        streamTbx = std::make_unique<TbxStream>();
    } else if (createTbxShmStream || createTbxShm4Stream) {
        if (sharedMemoryInfo.sysMemBase == nullptr) {
            if (enableThrow) {
                throw std::logic_error("Trying to use shared memory but no shared memory for system memory given.");
            }
        } else {
            if (createTbxShmStream) {
                streamTbxShm = std::make_unique<TbxShmStream>(aub_stream::mode::tbxShm);
            } else {
                streamTbxShm = std::make_unique<TbxShmStream>(aub_stream::mode::tbxShm4);
            }
        }
    } else if (createTbxShm3Stream) {
        if (sharedMemoryInfo.sysMemBase != nullptr || sharedMemoryInfo.localMemBase != nullptr) {
            if (enableThrow) {
                if (sharedMemoryInfo.localMemBase != nullptr) {
                    throw std::logic_error("Trying to use shared memory v3 but legacy shared memory for system memory given.");
                } else {
                    throw std::logic_error("Trying to use shared memory v3 but legacy shared memory for local memory given.");
                }
            }
        } else {
            if (gpu->gfxCoreFamily <= CoreFamily::Gen12lp) {
                if (enableThrow) {
                    throw std::logic_error("Trying to use shared memory v3 for legacy core family.");
                }
            } else {
                physicalAddressAllocator = std::make_unique<PhysicalAddressAllocatorHeap>();

                streamTbxShm = std::make_unique<TbxShmStream>(aub_stream::mode::tbxShm3);
            }
        }
    }
    if (streamMode == aub_stream::mode::aubFileAndTbx) {
        streamAubTbx = std::make_unique<AubTbxStream>(*streamAub, *streamTbx);
    }
    if (streamMode == aub_stream::mode::aubFileAndShm || streamMode == aub_stream::mode::aubFileAndShm4) {
        streamAubShm = std::make_unique<AubShmStream>(*streamAub, *streamTbxShm);
    }
}

void AubManagerImp::open(const std::string &aubFileName) {
    if (streamMode == aub_stream::mode::aubFile || streamMode == aub_stream::mode::aubFileAndTbx ||
        streamMode == aub_stream::mode::aubFileAndShm || streamMode == aub_stream::mode::aubFileAndShm4) {
        streamAub->open(aubFileName.c_str());
        streamAub->init(stepping, *gpu);
        gpu->initializeGlobalMMIO(*streamAub, devicesCount, memoryBankSize, stepping);
        gpu->setMemoryBankSize(*streamAub, devicesCount, memoryBankSize);
        setCCSMode(ccsCount);
        gpu->injectMMIOs(*streamAub, devicesCount);
    }

    for (auto &ctxt : hwContexts) {
        ctxt->initialize();
    }
}

void AubManagerImp::closeSocket() {
    AubStream *stream = getStream();
    if (stream) {
        if ((streamMode == mode::tbx) || (streamMode == mode::aubFileAndTbx)) {
            if (streamTbx->socket) {
                streamTbx->socket->close();
            }
        } else if ((IsAnyTbxShmMode(streamMode)) ||
                   (streamMode == mode::aubFileAndShm) || (streamMode == mode::aubFileAndShm4)) {
            if (streamTbxShm->socket) {
                streamTbxShm->socket->close();
            }
        }
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
    if (streamMode == aub_stream::mode::null) {
        return true;
    }
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
    if (streamAubShm) {
        streamAubShm->pauseAubFileStream(onoff);
    }
}

void AubManagerImp::blockMemWritesViaTbx(bool onoff) {
    if (streamAubShm) {
        streamAubShm->blockMemWritesViaTbxStream(onoff);
    }
}

void AubManagerImp::addComment(const char *message) {
    AubStream *stream = getStream();
    if (stream) {
        stream->addComment(message);
    }
}

HardwareContext *AubManagerImp::createHardwareContext(uint32_t device, uint32_t engine, uint32_t flags) {
    HardwareContext *context = nullptr;
    if (streamMode == aub_stream::mode::null) {
        context = new NullHardwareContext();
    } else {
        auto &csTraits = gpu->getCommandStreamerHelper(device, static_cast<EngineType>(engine));
        AubStream *stream = getStream();

        ContextGroup *group = &groupContextHelper->contextGroups[device][csTraits.engineType][0];
        context = new HardwareContextImp(device, *stream, csTraits, *ggtts[device], *ppgtts[device], group, flags);
    }

    std::lock_guard<std::mutex> lock(hwContextsMutex);
    hwContexts.push_back(context);

    return context;
}
HardwareContext *AubManagerImp::createHardwareContext2(const CreateHardwareContext2Params &params, uint32_t device, uint32_t engine, uint32_t flags) {
    HardwareContext *context = nullptr;

    if (streamMode == aub_stream::mode::null) {
        context = new NullHardwareContext();
    } else {
        auto &csTraits = gpu->getCommandStreamerHelper(device, static_cast<EngineType>(engine));
        AubStream *stream = getStream();

        ContextGroup *group = nullptr;

        if (flags & hardwareContextFlags::contextGroup) {
            auto groupId = 0u;

            if (params.primaryContextId == hardwareContextId::invalidContextId) {
                groupId = groupContextHelper->groupIds[device][csTraits.engineType];
                groupContextHelper->primaryContextIdToGroupId[device][csTraits.engineType][params.contextId] = groupId;

                if (groupId >= groupContextHelper->contextGroups[device][csTraits.engineType].size()) {
                    auto contextGroupCount = this->gpu->getContextGroupCount();
                    groupContextHelper->contextGroups[device][csTraits.engineType].resize(groupId + 1);
                    group = &groupContextHelper->contextGroups[device][csTraits.engineType][groupId];
                    group->contexts.resize(contextGroupCount);
                }
                groupContextHelper->groupIds[device][csTraits.engineType]++;
            } else {
                groupId = groupContextHelper->primaryContextIdToGroupId[device][csTraits.engineType][params.primaryContextId];
            }

            group = &groupContextHelper->contextGroups[device][csTraits.engineType][groupId];
        }
        context = new HardwareContextImp(device, *stream, csTraits, *ggtts[device], *ppgtts[device], group, flags);
    }

    std::lock_guard<std::mutex> lock(hwContextsMutex);
    hwContexts.push_back(context);

    return context;
}

bool AubManagerImp::releaseHardwareContext(HardwareContext *context) {
    bool contextFound = false;
    {
        std::lock_guard<std::mutex> lock(hwContextsMutex);

        auto iter = std::find(hwContexts.begin(), hwContexts.end(), context);
        if (iter != hwContexts.end()) {
            hwContexts.erase(iter);
            contextFound = true;
        }
    }

    if (contextFound) {
        delete context;
        return true;
    }
    return false;
}

void AubManagerImp::adjustPageSize(uint32_t memoryBanks, size_t &pageSize) {
    auto &csTraits = gpu->getCommandStreamerHelper(0, static_cast<EngineType>(ENGINE_CCS));

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
    if (streamMode == aub_stream::mode::null) {
        return;
    }
    adjustPageSize(allocationParams.memoryBanks, allocationParams.pageSize);
    AubStream *stream = getStream();

    auto pageTableEntries = stream->writeMemory(ppgtts[0].get(), allocationParams);

    for (uint32_t i = 1; i < ppgtts.size(); i++) {
        stream->cloneMemory(ppgtts[i].get(), pageTableEntries, allocationParams);
    }
}

void AubManagerImp::writePageTableEntries(uint64_t gfxAddress, size_t size, uint32_t memoryBanks,
                                          int hint, std::vector<PageInfo> &lastLevelPages, size_t pageSize) {
    if (streamMode == aub_stream::mode::null) {
        return;
    }

    adjustPageSize(memoryBanks, pageSize);
    AubStream *stream = getStream();

    auto pageTableEntries = stream->writeMemory(ppgtts[0].get(), AllocationParams(gfxAddress, nullptr, size, memoryBanks, hint, pageSize));

    for (uint32_t i = 1; i < ppgtts.size(); i++) {
        stream->cloneMemory(ppgtts[i].get(), pageTableEntries, AllocationParams(gfxAddress, nullptr, size, memoryBanks, 0, pageSize));
    }

    lastLevelPages.insert(lastLevelPages.end(), pageTableEntries.begin(), pageTableEntries.end());
}

void AubManagerImp::writePhysicalMemoryPages(const void *memory, std::vector<PageInfo> &pages, size_t size, int hint) {

    if (streamMode == aub_stream::mode::null) {
        return;
    }
    assert(pages.size() >= 1);
    AubStream *stream = getStream();
    stream->writePhysicalMemoryPages(memory, size, pages, hint);
}

void AubManagerImp::freeMemory(uint64_t gfxAddress, size_t size) {
    if (streamMode == aub_stream::mode::null) {
        return;
    }
    AubStream *stream = getStream();

    for (auto &ppgtt : ppgtts) {
        stream->freeMemory(ppgtt.get(), gfxAddress, size);
    }
}

bool AubManagerImp::reservePhysicalMemory(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) {
    if (streamMode == aub_stream::mode::null) {
        return true;
    }
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

bool AubManagerImp::reserveOnlyPhysicalSpace(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) {
    if (streamMode == aub_stream::mode::null) {
        return true;
    }
    adjustPageSize(allocationParams.memoryBanks, allocationParams.pageSize);
    auto size = allocationParams.size;
    auto memoryBanks = allocationParams.memoryBanks;

    if (streamMode != mode::tbxShm4 && streamMode != mode::aubFileAndShm4) {
        return false;
    }

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
    physicalAllocInfo.physicalAddress = static_cast<PhysicalAddressAllocatorSimpleAndSHM4Mapper *>(physicalAddressAllocator.get())->reserveOnlyPhysicalSpace(allocationParams.memoryBanks, sizePageAligned, pageSize);
    return true;
}

bool AubManagerImp::mapSystemMemoryToPhysicalAddress(uint64_t physAddress, size_t size, size_t alignment, bool isLocalMemory, const void *p) {
    if (streamMode != mode::tbxShm4 && streamMode != mode::aubFileAndShm4) {
        throwErrorIfEnabled("mapSystemMemoryToPhysicalAddress is not supported for this stream mode.");
        return false;
    }
    static_cast<PhysicalAddressAllocatorSimpleAndSHM4Mapper *>(physicalAddressAllocator.get())->mapSystemMemoryToPhysicalAddress(physAddress, size, alignment, isLocalMemory, p);
    return true;
}

void *AubManagerImp::translatePhysicalAddressToSystemMemory(uint64_t physicalAddress, bool isLocalMemory) {
    void *p = nullptr;
    switch (streamMode) {
    case mode::tbxShm4:
    case mode::aubFileAndShm4:
        size_t s;
        static_cast<PhysicalAddressAllocatorSimpleAndSHM4Mapper *>(physicalAddressAllocator.get())->translatePhysicalAddressToSystemMemory(physicalAddress, 0x1000, isLocalMemory, p, s);
        break;
    case mode::tbxShm3:
        p = reinterpret_cast<void *>(physicalAddress);
        break;
    case mode::tbxShm:
    case mode::aubFileAndShm:
        p = (isLocalMemory ? sharedMemoryInfo.localMemBase : sharedMemoryInfo.sysMemBase) + physicalAddress;
        break;
    default:
        throwErrorIfEnabled("translatePhysicalAddressToSystemMemory is not supported for this stream mode.");
        break;
    }
    return p;
}

bool AubManagerImp::mapGpuVa(uint64_t gfxAddress, size_t size, const PhysicalAllocationInfo physicalAllocInfo) {
    AllocationParams allocationParams(gfxAddress, nullptr, size, physicalAllocInfo.memoryBank, 0, physicalAllocInfo.pageSize);

    return mapGpuVa2(physicalAllocInfo.physicalAddress, allocationParams);
}

bool AubManagerImp::mapGpuVa2(uint64_t physicalAddress, AllocationParams params) {
    AubStream *stream = getStream();

    for (auto &ppgtt : ppgtts) {
        stream->mapGpuVa(ppgtt.get(), params, physicalAddress);
    }

    return true;
}

AubStream *AubManagerImp::getStream() const {
    AubStream *stream = nullptr;

    if (streamMode == mode::aubFile) {
        stream = streamAub.get();
    } else if (streamMode == mode::tbx) {
        stream = streamTbx.get();
    } else if (streamMode == mode::aubFileAndTbx) {
        stream = streamAubTbx.get();
    } else if (IsAnyTbxShmMode(streamMode)) {
        stream = streamTbxShm.get();
    } else if (streamMode == mode::aubFileAndShm || streamMode == mode::aubFileAndShm4) {
        stream = streamAubShm.get();
    } else {
        throwErrorIfEnabled("Stream is null.");
    }

    return stream;
}

AubManager *AubManager::create(const struct AubManagerOptions &options) {
    std::unique_ptr<Gpu> gpu;
    std::unique_ptr<Settings> settings;

    if (globalSettings == nullptr) {
        settings = std::make_unique<Settings>();
        globalSettings = settings.get();
    }

    if (options.version == 1) {
        auto createGpuFunc = getGpu(static_cast<ProductFamily>(options.productFamily));
        if (createGpuFunc) {
            gpu = createGpuFunc();
        }
    }
    if (nullptr != gpu) {
        auto aubManager = new AubManagerImp(std::move(gpu), options);
        aubManager->initialize();
        aubManager->setSettings(std::move(settings));
        if (aubManager->isInitialized()) {
            return aubManager;
        }
        delete aubManager;
    }
    return nullptr;
}

void AubManagerImp::writeMMIO(uint32_t offset, uint32_t value) {
    if (streamMode == aub_stream::mode::null) {
        return;
    }
    AubStream *stream = getStream();
    stream->writeMMIO(offset, value);
}

void AubManagerImp::writePCICFG(uint32_t offset, uint32_t value) {
    if (streamMode == aub_stream::mode::null) {
        return;
    }
    AubStream *stream = getStream();
    stream->writePCICFG(offset, value);
}

uint32_t AubManagerImp::readPCICFG(uint32_t offset) {
    if (streamMode == aub_stream::mode::null) {
        return 0;
    }
    AubStream *stream = getStream();
    return stream->readPCICFG(offset);
}

uint32_t AubManagerImp::readMMIO(uint32_t offset) {
    if (streamMode == aub_stream::mode::null) {
        return 0;
    }
    AubStream *stream = getStream();
    return stream->readMMIO(offset);
}

void AubManagerImp::setCCSMode(uint32_t ccsCount) {
    if (!hwContexts.empty()) {
        if (enableThrow) {
            throw std::logic_error("Cannot set CCS mode after hardware contexts have been created.");
        }
        return;
    }

    this->ccsCount = ccsCount;
    uint32_t mmioDevice = 0;
    uint32_t mmioCCSValue = 0;

    if (ccsCount <= 1) {
        mmioCCSValue = 0xFFF0000;
    } else if (ccsCount == 2) {
        mmioCCSValue = 0xFFF0208;
    } else {
        if (ccsCount > 4) {
            PRINT_LOG_INFO("Unsupported number of CCS provided: %d, using value of 4 CCS", ccsCount);
        }
        mmioCCSValue = 0xFFF0688;
    }

    for (uint32_t device = 0; device < devicesCount; device++) {
        writeMMIO(mmioDevice + 0x14804, mmioCCSValue);
        mmioDevice += mmioDeviceOffset;
    }
}

void AubManagerImp::throwErrorIfEnabled(const std::string &str) const {
    if (enableThrow) {
        throw std::runtime_error(str);
    }
}

void AubManagerImp::setSettings(std::unique_ptr<Settings> settingsIn) {
    settings = std::move(settingsIn);
}
bool AubManagerImp::isInitialized() const {
    return streamMode == aub_stream::mode::null || getStream() != nullptr;
}

} // namespace aub_stream
