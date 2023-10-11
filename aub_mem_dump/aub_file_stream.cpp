/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_file_stream.h"
#include "aub_mem_dump/gpu.h"
#include "aubstream/aubstream.h"
#include "gfx_core_family.h"
#include "options.h"

#include <cassert>
#include <string.h>
#include <algorithm>

namespace aub_stream {

const size_t g_dwordCountMax = 65536;

AubFileStream::~AubFileStream() {
    if (fileHandle.is_open()) {
        fileHandle.close();
    }
}

void AubFileStream::readDiscontiguousPages(void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable) {
}

void AubFileStream::addComment(const char *message) {
    CmdServicesMemTraceComment cmd = {};
    cmd.setHeader();
    cmd.syncOnComment = false;
    cmd.syncOnSimulatorDisplay = false;

    auto messageLen = strlen(message) + 1;
    auto dwordLen = ((messageLen + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1)) / sizeof(uint32_t);
    cmd.dwordCount = static_cast<uint32_t>(dwordLen + 1);

    fileHandle.write(reinterpret_cast<char *>(&cmd), sizeof(cmd) - sizeof(cmd.comment));
    fileHandle.write(message, messageLen);
    auto remainder = messageLen & (sizeof(uint32_t) - 1);
    if (remainder) {
        // if size is not 4 byte aligned, write extra zeros to AUB
        uint32_t zero = 0;
        fileHandle.write(reinterpret_cast<char *>(&zero), sizeof(uint32_t) - remainder);
    }
    fileHandle.flush();
}

void AubFileStream::declareContextForDumping(uint32_t handleDumpContext, PageTable *ppgtt) {
    AubPpgttContextCreate cmd = {};
    cmd.Header.Type = 0x7;
    cmd.Header.Opcode = 0x1;
    cmd.Header.SubOp = 0x14;
    cmd.Header.DwordLength = ((sizeof(cmd) - sizeof(cmd.Header)) / sizeof(uint32_t)) - 1;
    cmd.Handle = handleDumpContext;
    cmd.AdvancedContext = false;

    cmd.SixtyFourBit = ppgtt->getNumAddressBits() >= 48;
    if (cmd.SixtyFourBit) {
        cmd.PageDirPointer[0] = ppgtt->getPhysicalAddress();
    } else {
        auto entry = ppgtt->getChild(0);
        cmd.PageDirPointer[0] = entry ? entry->getPhysicalAddress() : 0;
        entry = ppgtt->getChild(1);
        cmd.PageDirPointer[1] = entry ? entry->getPhysicalAddress() : 0;
        entry = ppgtt->getChild(2);
        cmd.PageDirPointer[2] = entry ? entry->getPhysicalAddress() : 0;
        entry = ppgtt->getChild(3);
        cmd.PageDirPointer[3] = entry ? entry->getPhysicalAddress() : 0;
    }

    fileHandle.write((char *)&cmd, sizeof(cmd));
    fileHandle.flush();
}

void AubFileStream::dumpBufferBIN(AubStream::PageTableType gttType, uint64_t gfxAddress, size_t size, uint32_t handleDumpContext) {
    if (!dumpBinSupported) {
        return;
    }
    AubCaptureBinaryDumpHD cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.Header.Type = 0x7;
    cmd.Header.Opcode = 0x1;
    cmd.Header.SubOp = 0x15;
    cmd.Header.DwordLength = ((sizeof(cmd) - sizeof(cmd.Header)) / sizeof(uint32_t)) - 1;

    cmd.setHeight(1);
    cmd.setWidth(size);
    cmd.setBaseAddr(gfxAddress);
    cmd.setPitch(size);
    cmd.GttType = gttType;
    cmd.DirectoryHandle = handleDumpContext;

    fileHandle.write(reinterpret_cast<char *>(&cmd), sizeof(cmd));
    fileHandle.flush();
}

void AubFileStream::dumpSurface(PageTableType gttType, const SurfaceInfo &surfaceInfo, uint32_t handleDumpContext) {
    if (!dumpSurfaceSupported) {
        return;
    }
    CmdServicesMemTraceDumpCompress cmd;
    memset(&cmd, 0, sizeof(CmdServicesMemTraceDumpCompress));

    cmd.dwordCount = (sizeof(CmdServicesMemTraceDumpCompress) - 1) / 4;
    cmd.instructionSubOpcode = 0x10;
    cmd.instructionOpcode = 0x2e;
    cmd.instructionType = 0x7;

    cmd.setSurfaceAddress(surfaceInfo.address);
    cmd.surfaceWidth = surfaceInfo.width;
    cmd.surfaceHeight = surfaceInfo.height;
    cmd.surfacePitch = surfaceInfo.pitch;
    cmd.surfaceFormat = surfaceInfo.format;
    cmd.surfaceTilingType = surfaceInfo.tilingType;
    cmd.surfaceType = surfaceInfo.surftype;
    if (surfaceInfo.compressed) {
        cmd.algorithm = CmdServicesMemTraceDumpCompress::AlgorithmValues::Lossless;
        cmd.mode = CmdServicesMemTraceDumpCompress::ModeValues::Horizontal;
    } else {
        cmd.algorithm = CmdServicesMemTraceDumpCompress::AlgorithmValues::Uncompressed;
    }
    cmd.dumpType = surfaceInfo.dumpType;

    cmd.gttType = gttType;
    cmd.directoryHandle = handleDumpContext;

    cmd.useClearValue = surfaceInfo.useClearValue;

    cmd.clearColorType = surfaceInfo.clearColorType;
    cmd.clearColorAddress = surfaceInfo.clearColorAddress;
    cmd.auxSurfaceAddress = surfaceInfo.auxSurfaceAddress;
    cmd.auxSurfaceWidth = surfaceInfo.auxSurfaceWidth;
    cmd.auxSurfaceHeight = surfaceInfo.auxSurfaceHeight;
    cmd.auxSurfacePitch = surfaceInfo.auxSurfacePitch;
    cmd.auxSurfaceQPitch = surfaceInfo.auxSurfaceQPitch;
    cmd.auxSurfaceTilingType = surfaceInfo.auxSurfaceTilingType;
    cmd.auxEncodingFormat = surfaceInfo.auxEncodingFormat;

    fileHandle.write(reinterpret_cast<char *>(&cmd), sizeof(cmd));
    fileHandle.flush();
}

bool AubFileStream::init(int stepping, const GpuDescriptor &gpu) {
    this->dumpBinSupported = gpu.gfxCoreFamily >= CoreFamily::XeHpCore;
    this->dumpSurfaceSupported = gpu.gfxCoreFamily >= CoreFamily::XeHpCore;

    CmdServicesMemTraceVersion header = {};

    header.setHeader();
    header.dwordCount = (sizeof(header) / sizeof(uint32_t)) - 1;
    header.stepping = stepping;
    header.metal = 0;
    header.device = gpu.deviceId;
    header.csxSwizzling = CmdServicesMemTraceVersion::CsxSwizzlingValues::Disabled;
    // Which recording method used:
    //  Phys is required for GGTT memory to be written directly to phys vs through aperture.
    header.recordingMethod = CmdServicesMemTraceVersion::RecordingMethodValues::Phy;
    header.pch = CmdServicesMemTraceVersion::PchValues::Default;
    header.captureTool = CmdServicesMemTraceVersion::CaptureToolValues::AubStream;
    memset(header.deviceAbbreviation, 0, 8);
    if (gpu.deviceId == 0) {
        memcpy(header.deviceAbbreviation, gpu.productAbbreviation.c_str(), std::min(size_t(8), gpu.productAbbreviation.length()));
    }
    getHeaderStr(aubStreamCaller, header.commandLine);

    fileHandle.write(reinterpret_cast<char *>(&header), sizeof(header));
    fileHandle.flush();
    return true;
}

void getHeaderStr(uint32_t caller, char *header) {
    switch (caller) {
    case caller::rlr:
        memcpy(header, "RLR", 4);
        break;
    case caller::rlc:
        memcpy(header, "RLC", 4);
        break;
    case caller::rll:
        memcpy(header, "RLL", 4);
        break;
    case caller::rl:
        memcpy(header, "RL", 3);
        break;
    case caller::neo:
        memcpy(header, "NEO", 4);
        break;
    default:
        memcpy(header, "UNK", 4);
        break;
    }
}

void AubFileStream::expectMemoryTable(const void *memory, size_t size, const std::vector<PageInfo> &entries, uint32_t compareOperation) {
    size_t sizeWritten = 0;
    for (auto &entry : entries) {
        CmdServicesMemTraceMemoryCompare cmd = {};
        cmd.setHeader();
        cmd.address = (uint32_t)entry.physicalAddress;
        cmd.addressHigh = (uint32_t)(entry.physicalAddress >> 32);
        cmd.noReadExpect = CmdServicesMemTraceMemoryCompare::NoReadExpectValues::ReadExpect;
        cmd.repeatMemory = CmdServicesMemTraceMemoryCompare::RepeatMemoryValues::NoRepeat;
        cmd.tiling = CmdServicesMemTraceMemoryCompare::TilingValues::NoTiling;
        cmd.crcCompare = CmdServicesMemTraceMemoryCompare::CrcCompareValues::NoCrc;
        cmd.compareOperation = compareOperation;
        cmd.dataTypeHint = CmdServicesMemTraceMemoryCompare::DataTypeHintValues::TraceNotype;
        cmd.addressSpace = entry.isLocalMemory
                               ? AddressSpaceValues::TraceLocal
                               : AddressSpaceValues::TraceNonlocal;
        cmd.deferCompareEvaluation = ((&entry == &entries.back()) || (compareOperation == CmdServicesMemTraceMemoryCompare::CompareOperationValues::CompareEqual))
                                         ? CmdServicesMemTraceMemoryCompare::DeferCompareEvaluationValues::CompareImmediately
                                         : CmdServicesMemTraceMemoryCompare::DeferCompareEvaluationValues::StoreComparison;

        auto headerSize = sizeof(CmdServicesMemTraceMemoryCompare) - sizeof(CmdServicesMemTraceMemoryCompare::data);
        auto sizeThisIteration = entry.size;

        // Round up to the number of dwords
        auto dwordCount = (headerSize + sizeThisIteration + sizeof(uint32_t) - 1) / sizeof(uint32_t);

        cmd.dwordCount = static_cast<uint32_t>(dwordCount - 1);
        cmd.dataSizeInBytes = static_cast<uint32_t>(sizeThisIteration);

        fileHandle.write((char *)&cmd, headerSize);
        fileHandle.write((char *)memory, entry.size);

        auto remainder = entry.size & (sizeof(uint32_t) - 1);
        if (remainder) {
            uint32_t zero = 0;
            fileHandle.write((const char *)&zero, sizeof(uint32_t) - remainder);
        }

        memory = entry.size + (uint8_t *)memory;
        sizeWritten += entry.size;
    }

    fileHandle.flush();
    assert(sizeWritten == size);
}

void AubFileStream::reserveContiguousPages(const std::vector<uint64_t> &entries) {
    CmdServicesMemTraceMemoryContinuousRegion cmd = {};
    cmd.setHeader();
    cmd.dwordCount = (sizeof(cmd) / sizeof(uint32_t)) - 1;

    for (auto &page : entries) {
        // If a 64KB page, reserve that region
        cmd.regionSize = 0x10000;
        cmd.address = page;
        fileHandle.write((char *)&cmd, sizeof(cmd));
    }
    fileHandle.flush();
}

void AubFileStream::writeContiguousPages(const void *memory, size_t size, uint64_t physAddress, int addressSpace, int hint) {
    CmdServicesMemTraceMemoryWrite header = {};
    auto sizeMemoryWriteHeader = sizeof(CmdServicesMemTraceMemoryWrite) - sizeof(CmdServicesMemTraceMemoryWrite::data);
    auto alignedBlockSize = (size + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1);
    auto dwordCount = (sizeMemoryWriteHeader + alignedBlockSize) / sizeof(uint32_t);
    assert(dwordCount <= aub_stream::g_dwordCountMax);

    header.setHeader();
    header.dwordCount = static_cast<uint32_t>(dwordCount - 1);
    header.address = physAddress;
    header.repeatMemory = CmdServicesMemTraceMemoryWrite::RepeatMemoryValues::NoRepeat;
    header.tiling = CmdServicesMemTraceMemoryWrite::TilingValues::NoTiling;
    header.dataTypeHint = hint;
    header.addressSpace = addressSpace;
    header.dataSizeInBytes = static_cast<uint32_t>(size);

    fileHandle.write(reinterpret_cast<const char *>(&header), sizeMemoryWriteHeader);
    fileHandle.write((const char *)memory, size);

    auto remainder = size & (sizeof(uint32_t) - 1);
    if (remainder) {
        uint32_t zero = 0;
        fileHandle.write((const char *)&zero, sizeof(uint32_t) - remainder);
    }
    fileHandle.flush();
}

void AubFileStream::writeDiscontiguousPages(const void *memory, size_t size, const std::vector<PageInfo> &writeInfoTable, int hint) {
    auto maxEntries = sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::Dword_2_To_190) / sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::Dword_2_To_190[0]);
    bool isLocalMemory = writeInfoTable[0].isLocalMemory;
    int addressSpace = isLocalMemory
                           ? AddressSpaceValues::TraceLocal
                           : AddressSpaceValues::TraceNonlocal;

    if (writeInfoTable.size() == 1) {
        // Fall back to a simplified write if only one entry
        auto &entry = writeInfoTable[0];
        writeContiguousPages(memory, size, entry.physicalAddress, addressSpace, hint);
    } else {
        CmdServicesMemTraceMemoryWriteDiscontiguous cmd = {};
        cmd.setHeader();
        cmd.frontDoorAccess = 0;
        cmd.repeatMemory = CmdServicesMemTraceMemoryWriteDiscontiguous::RepeatMemoryValues::NoRepeat;
        cmd.tiling = CmdServicesMemTraceMemoryWriteDiscontiguous::TilingValues::NoTiling;
        cmd.dataTypeHint = hint;
        cmd.addressSpace = addressSpace;
        cmd.dwordCount = 0;

        auto index = 0u;
        size_t writtenSize = 0;
        auto itorCurrent = writeInfoTable.begin();
        auto itorDumpStart = itorCurrent;
        auto ptrDump = memory;
        auto ptr = memory;
        auto headerSize = sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous) - sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::data);

        // Loop for all entries
        while (itorCurrent != writeInfoTable.end()) {
            auto &writeInfo = *itorCurrent;
            auto dwordCount = uint32_t(writeInfo.size / sizeof(uint32_t));

            // Make sure we have room for another entry.
            if (index >= maxEntries || ((dwordCount + cmd.dwordCount) > 65535)) {
                // if not, flush out existing token
                cmd.numberOfAddressDataPairs = index;
                fileHandle.write((char *)&cmd, headerSize);

                // Write the data
                while (itorDumpStart != itorCurrent) {
                    bool unalignedSize = itorDumpStart->size & (sizeof(uint32_t) - 1);
                    bool unalignedAddress = itorDumpStart->physicalAddress & (sizeof(uint32_t) - 1);
                    bool differentAddressSpace = (isLocalMemory != itorDumpStart->isLocalMemory);
                    if (!unalignedSize && !unalignedAddress && !differentAddressSpace) {
                        fileHandle.write((const char *)ptrDump, itorDumpStart->size);
                    }

                    ptrDump = itorDumpStart->size + (uint8_t *)ptrDump;
                    ++itorDumpStart;
                }

                // reset for next token
                cmd.dwordCount = 0;
            }

            if (!cmd.dwordCount) {
                assert(itorDumpStart == itorCurrent);
                assert(ptrDump == ptr);
                cmd.dwordCount = (headerSize / sizeof(uint32_t)) - 1;
                index = 0;
            }

            bool unalignedSize = writeInfo.size & (sizeof(uint32_t) - 1);
            bool unalignedAddress = writeInfo.physicalAddress & (sizeof(uint32_t) - 1);
            bool differentAddressSpace = (isLocalMemory != writeInfo.isLocalMemory);
            if (unalignedSize || unalignedAddress || differentAddressSpace) {
                // If we're unaligned, fall back to simplified write
                writeContiguousPages(
                    ptr,
                    writeInfo.size,
                    writeInfo.physicalAddress,
                    writeInfo.isLocalMemory ? AddressSpaceValues::TraceLocal : AddressSpaceValues::TraceNonlocal,
                    hint);
            } else {
                // Store the entries in the command
                cmd.Dword_2_To_190[index].dataSizeInBytes = uint32_t(writeInfo.size);
                cmd.Dword_2_To_190[index].address = writeInfo.physicalAddress;

                cmd.dwordCount += dwordCount;

                ++index;
            }

            ptr = writeInfo.size + (uint8_t *)ptr;
            writtenSize += writeInfo.size;
            ++itorCurrent;
        }

        if (index) {
            assert(itorDumpStart != itorCurrent);

            // if not, flush out existing token
            cmd.numberOfAddressDataPairs = index;
            fileHandle.write((char *)&cmd, headerSize);

            // Write the data
            while (itorDumpStart != itorCurrent) {
                auto &writeInfo = *itorDumpStart;
                bool unalignedSize = writeInfo.size & (sizeof(uint32_t) - 1);
                bool unalignedAddress = writeInfo.physicalAddress & (sizeof(uint32_t) - 1);
                bool differentAddressSpace = (isLocalMemory != writeInfo.isLocalMemory);
                if (!unalignedSize && !unalignedAddress && !differentAddressSpace) {
                    fileHandle.write((const char *)ptrDump, writeInfo.size);
                }

                ptrDump = writeInfo.size + (uint8_t *)ptrDump;
                ++itorDumpStart;
            }
        }
        fileHandle.flush();
        assert(writtenSize == size);
    }
}

void AubFileStream::writeDiscontiguousPages(const std::vector<PageEntryInfo> &writeInfoTable, int addressSpace, int hint) {
    auto maxEntries = sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::Dword_2_To_190) / sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::Dword_2_To_190[0]);
    if (writeInfoTable.size() == 1) {
        // Fall back to a simplified write if only one entry
        auto &entry = writeInfoTable[0];
        writeContiguousPages(&entry.tableEntry, sizeof(entry.tableEntry), entry.physicalAddress, addressSpace, hint);
    } else {
        CmdServicesMemTraceMemoryWriteDiscontiguous cmd = {};
        cmd.setHeader();
        cmd.frontDoorAccess = 0;
        cmd.repeatMemory = CmdServicesMemTraceMemoryWriteDiscontiguous::RepeatMemoryValues::NoRepeat;
        cmd.tiling = CmdServicesMemTraceMemoryWriteDiscontiguous::TilingValues::NoTiling;
        cmd.dataTypeHint = hint;
        cmd.addressSpace = addressSpace;
        cmd.dwordCount = 0;

        auto index = 0u;
        auto itorCurrent = writeInfoTable.begin();
        auto itorDumpStart = itorCurrent;
        auto headerSize = sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous) - sizeof(CmdServicesMemTraceMemoryWriteDiscontiguous::data);

        // Loop for all entries
        while (itorCurrent != writeInfoTable.end()) {
            auto &writeInfo = *itorCurrent;
            auto dwordCount = uint32_t(sizeof(writeInfo.tableEntry) / sizeof(uint32_t));

            // Make sure we have room for another entry.
            if (index >= maxEntries || ((dwordCount + cmd.dwordCount) > 65535)) {
                // if not, flush out existing token
                cmd.numberOfAddressDataPairs = index;
                fileHandle.write((char *)&cmd, headerSize);

                // Write the data
                while (itorDumpStart != itorCurrent) {
                    fileHandle.write((const char *)&itorDumpStart->tableEntry, sizeof(itorDumpStart->tableEntry));
                    ++itorDumpStart;
                }

                // reset for next token
                cmd.dwordCount = 0;
            }

            if (!cmd.dwordCount) {
                assert(itorDumpStart == itorCurrent);
                cmd.dwordCount = (headerSize / sizeof(uint32_t)) - 1;
                index = 0;
            }

            // Store the entries in the command
            assert(index < maxEntries);
            cmd.Dword_2_To_190[index].dataSizeInBytes = sizeof(writeInfo.tableEntry);
            cmd.Dword_2_To_190[index].address = writeInfo.physicalAddress;

            cmd.dwordCount += dwordCount;

            ++index;
            ++itorCurrent;
        }

        if (index) {
            assert(itorDumpStart != itorCurrent);

            // if not, flush out existing token
            cmd.numberOfAddressDataPairs = index;
            fileHandle.write((char *)&cmd, headerSize);

            // Write the data
            while (itorDumpStart != itorCurrent) {
                auto &writeInfo = *itorDumpStart;
                fileHandle.write((const char *)&writeInfo.tableEntry, sizeof(writeInfo.tableEntry));
                ++itorDumpStart;
            }
        }
        fileHandle.flush();
    }
}

void AubFileStream::writeMMIO(uint32_t offset, uint32_t value) {
    CmdServicesMemTraceRegisterWrite header = {};
    header.setHeader();
    header.dwordCount = (sizeof(header) / sizeof(uint32_t)) - 1;
    header.registerOffset = offset;
    header.messageSourceId = CmdServicesMemTraceRegisterWrite::MessageSourceIdValues::Ia;
    header.registerSize = CmdServicesMemTraceRegisterWrite::RegisterSizeValues::Dword;
    header.registerSpace = CmdServicesMemTraceRegisterWrite::RegisterSpaceValues::Mmio;
    header.writeMaskLow = 0xffffffff;
    header.writeMaskHigh = 0x00000000;
    header.data[0] = value;

    fileHandle.write((char *)&header, sizeof(header));
    fileHandle.flush();
}

void AubFileStream::registerPoll(uint32_t registerOffset, uint32_t mask, uint32_t desiredValue, bool pollNotEqual, uint32_t timeoutAction) {
    CmdServicesMemTraceRegisterPoll header = {};
    header.setHeader();
    header.registerOffset = registerOffset;
    header.timeoutAction = timeoutAction;
    header.pollNotEqual = pollNotEqual;
    header.operationType = CmdServicesMemTraceRegisterPoll::OperationTypeValues::Normal;
    header.registerSize = CmdServicesMemTraceRegisterPoll::RegisterSizeValues::Dword;
    header.registerSpace = CmdServicesMemTraceRegisterPoll::RegisterSpaceValues::Mmio;
    header.pollMaskLow = mask;
    header.data[0] = desiredValue;
    header.dwordCount = (sizeof(header) / sizeof(uint32_t)) - 1;

    fileHandle.write((char *)&header, sizeof(header));
    fileHandle.flush();
}

void AubFileStream::open(const char *name) {
    fileHandle.open(name, std::ofstream::binary);
    fileName.assign(name);
}

void AubFileStream::close() {
    fileHandle.close();
    fileName.clear();
}

bool AubFileStream::isOpen() {
    return fileHandle.is_open();
}

const std::string &AubFileStream::getFileName() {
    return fileName;
}

void AubFileStream::writeGttPages(GGTT *ggtt, const std::vector<PageEntryInfo> &writeInfoTable) {
    writeDiscontiguousPages(writeInfoTable, AddressSpaceValues::TraceGttEntry, DataTypeHintValues::TraceNotype);
}

} // namespace aub_stream
