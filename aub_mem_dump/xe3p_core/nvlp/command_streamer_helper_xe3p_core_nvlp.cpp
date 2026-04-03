/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"
#include "aubstream/product_family.h"
#include "aub_mem_dump/page_table_pml5.h"

namespace aub_stream {

struct GpuNvlp : public GpuXe3pCore {
    static constexpr uint32_t numSupportedDevices = 1;

    GpuNvlp() {
        productFamily = ProductFamily::Nvlp;
        gfxCoreFamily = CoreFamily::Xe3pCore;
        productAbbreviation = "nvlp";
        deviceId = 0;
        deviceCount = GpuNvlp::numSupportedDevices;
    }

    uint64_t getWOPCMSize() const override { return 16 * 1024 * 1024; };

    const MMIOList getGlobalMMIOPlatformSpecific() const override {
        const MMIOList globalMMIOPlatformSpecific = {
            // bits: 8: IG_PAT, 7-6: L3_CLOS, 5-4: L3_CACHE_POLICY, 3-2:L4_CACHE_POLICY
            MMIOPair(0x00004000, 0b0000'0000'1100), // Defer to PAT
            MMIOPair(0x00004004, 0b0001'0000'1100), // L3
            MMIOPair(0x00004008, 0b0001'0011'0000), // L4
            MMIOPair(0x0000400C, 0b0001'0011'1100), // UC
            MMIOPair(0x00004010, 0b0001'0000'0000), // L3 + L4

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
            MMIOPair(getPatIndexMmioAddr(16), 0b0010'0000'1110), // L3 Compressed (1-Way Coherent)
            // PatIndex 17 Reserved
            MMIOPair(getPatIndexMmioAddr(18), 0b0100'0010'1100), // L3:XA
            MMIOPair(getPatIndexMmioAddr(19), 0b0100'0010'1110), // L3:XA (1-Way Coherent)
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

            MMIOPair(0x000047FC, 0b0000'0000'1111), // PAT_ATS: PCIe ATS/PASID
            MMIOPair(0x00004820, 0b0000'0000'0011), // PTA_MODE: PPGTT/TRTT
        };
        return globalMMIOPlatformSpecific;
    };

    void setGGTTBaseAddresses(AubStream &stream, uint32_t deviceCount, uint64_t memoryBankSize) const override {
        assert(deviceCount > 0u);
        assert(deviceCount <= this->deviceCount);
        assert(GpuNvlp::numSupportedDevices == this->deviceCount);

        const uint32_t mmioDevice[1] = {0};
        const uint32_t gsmBase = 0x108100;

        uint64_t gttBase = getGSMBaseAddress(0);
        stream.writeMMIO(mmioDevice[0] + gsmBase + 4, static_cast<uint32_t>(gttBase >> 32));
        stream.writeMMIO(mmioDevice[0] + gsmBase + 0, static_cast<uint32_t>(gttBase & 0xFFF00000));
    }

    bool isMemorySupported(uint32_t memoryBanks, uint32_t alignment) const override {
        assert(MEMORY_BANK_SYSTEM == 0);
        auto supportedBanks = MEMORY_BANK_SYSTEM;
        auto unsupportedBanks = MEMORY_BANKS_ALL ^ supportedBanks;

        if (unsupportedBanks & memoryBanks) {
            return false;
        }

        // MEMORY_BANK_SYSTEM
        return alignment == 4096 || alignment == 65536;
    }

    void initializeFlatCcsBaseAddressMmio(AubStream &stream, uint32_t deviceIndex, uint64_t flatCcsBaseAddress, uint64_t size) const override {

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
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Nvlp>() {
    return std::make_unique<GpuNvlp>;
}
} // namespace aub_stream
