/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "aubstream/aub_manager.h"
#include "aubstream/page_info.h"
#include "aubstream/allocation_params.h"
#include "page_table.h"
#include "aubstream/shared_mem_info.h"
#include <memory>
#include <string>
#include <vector>

namespace aub_stream {

struct AubFileStream;
struct AubStream;
struct TbxStream;
struct AubTbxStream;
struct TbxShmStream;

struct Gpu;
struct StolenMemory;
struct PageInfo;
struct HardwareContext;
struct HardwareContextImp;
struct PhysicalAddressAllocator;

class AubManagerImp : public AubManager {
  public:
    AubManagerImp(const Gpu &gpu, const struct AubManagerOptions &options);

    ~AubManagerImp();

    AubManagerImp &operator=(const AubManagerImp &) = delete;
    AubManagerImp(const AubManagerImp &) = delete;

    HardwareContext *createHardwareContext(uint32_t device, uint32_t engine, uint32_t flags) override;

    void open(const std::string &aubFileName) override;
    void close() override;
    bool isOpen() override;
    const std::string getFileName() override;
    void pause(bool onoff) override;
    bool isInitialized() {
        return getStream() != nullptr;
    }

    void addComment(const char *message) override;
    void writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) override;
    void writeMemory2(AllocationParams allocationParams) override;
    void writePageTableEntries(uint64_t gfxAddress, size_t size, uint32_t memoryBanks, int hint,
                               std::vector<PageInfo> &lastLevelPages, size_t pageSize) override;

    void writePhysicalMemoryPages(const void *memory, std::vector<PageInfo> &pages, size_t size, int hint) override;
    void freeMemory(uint64_t gfxAddress, size_t size) override;

    virtual bool reservePhysicalMemory(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) override;
    virtual bool mapGpuVa(uint64_t gfxAddress, size_t size, PhysicalAllocationInfo physicalAllocInfo) override;

  protected:
    void initialize();
    AubStream *getStream();
    void adjustPageSize(uint32_t memoryBanks, size_t &pageSize);
    void throwErrorIfEnabled(const std::string &);

    const Gpu &gpu;
    const uint32_t devicesCount;
    const uint64_t memoryBankSize;
    const bool localMemorySupported;
    const uint32_t streamMode;
    const uint32_t stepping;
    uint64_t gpuAddressSpace;

    std::unique_ptr<StolenMemory> stolenMem;
    std::unique_ptr<AubFileStream> streamAub;
    std::unique_ptr<TbxStream> streamTbx;
    std::unique_ptr<AubTbxStream> streamAubTbx;
    std::unique_ptr<TbxShmStream> streamTbxShm;

    std::unique_ptr<PhysicalAddressAllocator> physicalAddressAllocator;

    std::vector<std::unique_ptr<PageTable>> ppgtts;
    std::vector<std::unique_ptr<GGTT>> ggtts;
    std::vector<HardwareContextImp *> hwContexts;

    SharedMemoryInfo sharedMemoryInfo;
    const bool enableThrow{};
};

} // namespace aub_stream
