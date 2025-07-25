/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/page_table_pml5.h"
#include "aub_mem_dump/xe_hpc_core/command_streamer_helper_xe_hpc_core.h"

#include "aubstream/product_family.h"

namespace aub_stream {
struct GpuPvc : public GpuXeHpcCore {
    GpuPvc() {
        productFamily = ProductFamily::Pvc;
        gfxCoreFamily = CoreFamily::XeHpcCore;
        productAbbreviation = "pvc";
        deviceId = 0x27;
        deviceCount = GpuXeHpcCore::numSupportedDevices;
    }

    const MMIOList getGlobalMMIO() const override {
        const MMIOList globalMMIO = {
            MMIOPair(0x00002090, 0xffff0000), // CHICKEN_PWR_CTX_RASTER_1
            MMIOPair(0x000020d8, 0x00020000), // CS_DEBUG_MODE2_RCSUNIT
            MMIOPair(0x000020e0, 0xffff4000), // FF_SLICE_CS_CHICKEN1_RCSUNIT
            MMIOPair(0x000020e4, 0xffff0000), // FF_SLICE_CS_CHICKEN2_RCSUNIT
            MMIOPair(0x000020ec, 0xffff0051), // CS_DEBUG_MODE1
            MMIOPair(0x00002580, 0xffff0005), // CS_CHICKEN1

            // GLOBAL_MOCS
            MMIOPair(0x00004000, 0x00000008),
            MMIOPair(0x00004004, 0x00000038),
            MMIOPair(0x00004008, 0x00000038),
            MMIOPair(0x0000400C, 0x00000008),
            MMIOPair(0x00004010, 0x00000018),
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

            // LNCF_MOCS (PVC, ...)
            MMIOPair(0x0000B020, 0x00100000), // LNCF0: Lower - Error (Reserved for Non-Use),  Upper - UC
            MMIOPair(0x0000B024, 0x00500030), // LNCF1: Lower - L3
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
            MMIOPair(0x0000b134, 0xa0000000), // L3ALLOCREG
            MMIOPair(0x0000b234, 0xa0000000), // L3ALLOCREG_CCS0
            MMIOPair(0x0000ce90, 0x00030003), // GFX_MULT_CTXT_CTL
            MMIOPair(0x0000cf58, 0x80000000), // LMEM_CFG for local memory
            MMIOPair(0x0000e194, 0xffff0002), // CHICKEN_SAMPLER_2
            MMIOPair(0x00014800, 0x00030003), // RCU_MODE
            MMIOPair(0x00014804, 0x0fff0000), // CCS_MODE
            MMIOPair(0x0001a0d8, 0x00020000), // CS_DEBUG_MODE2_CCSUNIT
            MMIOPair(0x00042080, 0x00000000), // CHICKEN_MISC_1
        };
        return globalMMIO;
    }

    void initializeGlobalMMIO(AubStream &stream, uint32_t devicesCount, uint64_t memoryBankSize, uint32_t stepping) const override {
        uint32_t mmioDevice = 0;

        for (uint32_t device = 0; device < devicesCount; device++) {
            const auto &globalMMIO = getGlobalMMIO();
            for (const auto &mmioPair : globalMMIO) {
                stream.writeMMIO(mmioDevice + mmioPair.first, mmioPair.second);
            }

            if (stepping == SteppingValues::A) {
                stream.writeMMIO(mmioDevice + 0x0000b120, 0x54000002); // LTCDREG
            } else {
                stream.writeMMIO(mmioDevice + 0x0000b120, 0x14000002); // LTCDREG
            }

            mmioDevice += mmioDeviceOffset;
        }
    }
};

template <>
std::function<std::unique_ptr<Gpu>()> enableGpu<ProductFamily::Pvc>() {
    return std::make_unique<GpuPvc>;
}
} // namespace aub_stream
