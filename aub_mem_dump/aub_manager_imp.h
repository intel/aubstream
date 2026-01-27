/*
 * Copyright (C) 2022-2026 Intel Corporation
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
struct AubShmStream;
struct Gpu;
struct StolenMemory;
struct PageInfo;
struct HardwareContext;
struct HardwareContextImp;
struct GroupContextHelper;
struct PhysicalAddressAllocator;
class Settings;

class AubManagerImp : public AubManager {
  public:
    AubManagerImp(std::unique_ptr<Gpu> gpu, const struct AubManagerOptions &options);

    ~AubManagerImp();

    AubManagerImp &operator=(const AubManagerImp &) = delete;
    AubManagerImp(const AubManagerImp &) = delete;

    HardwareContext *createHardwareContext(uint32_t device, uint32_t engine, uint32_t flags) override;
    HardwareContext *createHardwareContext2(const CreateHardwareContext2Params &params, uint32_t device, uint32_t engine, uint32_t flags) override;
    HardwareContext *createHardwareContext3(const HardwareContextParamsHeader *params) override;

    void open(const std::string &aubFileName) override;
    void close() override;
    bool isOpen() override;
    const std::string getFileName() override;
    void pause(bool onoff) override;
    void blockMemWritesViaTbx(bool onoff) override;
    bool isInitialized() const;

    void addComment(const char *message) override;
    void writeMemory(uint64_t gfxAddress, const void *memory, size_t size, uint32_t memoryBanks, int hint, size_t pageSize) override;
    void writeMemory2(AllocationParams allocationParams) override;
    void writePageTableEntries(uint64_t gfxAddress, size_t size, uint32_t memoryBanks, int hint,
                               std::vector<PageInfo> &lastLevelPages, size_t pageSize) override;

    void writePhysicalMemoryPages(const void *memory, std::vector<PageInfo> &pages, size_t size, int hint) override;
    void freeMemory(uint64_t gfxAddress, size_t size) override;

    bool reservePhysicalMemory(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) override;
    bool reserveOnlyPhysicalSpace(AllocationParams allocationParams, PhysicalAllocationInfo &physicalAllocInfo) override;
    bool mapGpuVa(uint64_t gfxAddress, size_t size, PhysicalAllocationInfo physicalAllocInfo) override;
    bool mapGpuVa2(uint64_t physicalAddress, AllocationParams params) override;
    bool mapSystemMemoryToPhysicalAddress(uint64_t physAddress, size_t size, size_t alignment, bool isLocalMemory, const void *p) override;
    void *translatePhysicalAddressToSystemMemory(uint64_t physicalAddress, bool isLocalMemory) override;
    void initialize();
    void setSettings(std::unique_ptr<Settings> settingsIn);
    GroupContextHelper *getGroupContextHelper() {
        return groupContextHelper.get();
    }
    void writePCICFG(uint32_t offset, uint32_t value) override;
    uint32_t readPCICFG(uint32_t offset) override;
    uint32_t readMMIO(uint32_t offset) override;
    void writeMMIO(uint32_t offset, uint32_t value) override;
    void setCCSMode(uint32_t ccsCount) override;

    bool releaseHardwareContext(HardwareContext *context) override;

    void closeSocket(void) override;

  protected:
    virtual void createStream();
    AubStream *getStream() const;
    void throwErrorIfEnabled(const std::string &) const;

    std::unique_ptr<Gpu> gpu;
    const uint32_t devicesCount;
    const uint64_t memoryBankSize;
    const bool localMemorySupported;
    const uint32_t streamMode;
    const uint32_t stepping;
    uint64_t gpuAddressSpace;
    uint32_t ccsCount{};

    std::unique_ptr<AubFileStream> streamAub;
    std::unique_ptr<TbxStream> streamTbx;
    std::unique_ptr<AubTbxStream> streamAubTbx;
    std::unique_ptr<TbxShmStream> streamTbxShm;
    std::unique_ptr<AubShmStream> streamAubShm;

    std::unique_ptr<PhysicalAddressAllocator> physicalAddressAllocator;

    std::vector<std::unique_ptr<PageTable>> ppgtts;
    std::vector<std::unique_ptr<GGTT>> ggtts;
    std::vector<HardwareContext *> hwContexts;
    std::mutex hwContextsMutex{};

    SharedMemoryInfo sharedMemoryInfo;
    const bool enableThrow{};

    std::unique_ptr<Settings> settings;
    std::unique_ptr<GroupContextHelper> groupContextHelper;
};

} // namespace aub_stream
