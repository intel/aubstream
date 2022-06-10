/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/family_mapper.h"
#include "aub_mem_dump/memory_banks.h"
#include "gmock/gmock.h"
#include "test_defaults.h"
#include "tests/test_traits/test_traits.h"
#include <algorithm>
#include <stdio.h>

using namespace aub_stream;

extern std::string getTestFilter(const Gpu &gpu);

int runTests(const Gpu *gpu) {
    int result = 0;

    auto traits = testTraits[gpu->productFamily];

    if (gpu->gfxCoreFamily >= GEN12_CORE) {
        defaultEngine = EngineType::ENGINE_CCS;
    } else {
        defaultEngine = EngineType::ENGINE_RCS;
    }
    defaultDevice = std::min(defaultDevice, gpu->deviceCount - 1);

    // Default to 4KB sys
    defaultMemoryBank = MEMORY_BANK_SYSTEM;
    defaultPageSize = 4096u;

    // Attempt to promote to 64KB sys
    if (gpu->isMemorySupported(defaultMemoryBank, 65536u)) {
        defaultPageSize = 65536u;
    }

    // Attempt to promote to 4KB local
    if (gpu->isMemorySupported(MEMORY_BANK_0, 4096u)) {
        defaultMemoryBank = MEMORY_BANK_0;
        defaultPageSize = 4096u;
    }

    // Attempt to promote to 64KB local
    if (gpu->isMemorySupported(MEMORY_BANK_0, 65536u)) {
        defaultMemoryBank = MEMORY_BANK_0;
        defaultPageSize = 65536u;
    }

    std::cout << "\n---------------------------------------------------\n";
    std::cout << "\nRunning tests for: "
              << gpu->productAbbreviation << "_" << traits->deviceSliceCount << "x" << traits->deviceSubSliceCount << "x" << traits->deviceEuPerSubSlice << std::endl;
    std::cout << "\n---------------------------------------------------\n";

    std::string filter = getTestFilter(*gpu);
    auto initialFilter = ::testing::GTEST_FLAG(filter);

    if (filter.size()) {
        ::testing::GTEST_FLAG(filter) += "*:-";
        ::testing::GTEST_FLAG(filter) += filter;
    }
    result = RUN_ALL_TESTS();

    ::testing::GTEST_FLAG(filter) = initialFilter;

    std::cout << "\n---------------------------------------------------\n";
    std::cout << "\nTests returned: " << result << std::endl;
    std::cout << "\n---------------------------------------------------\n";

    return result;
}

int main(int argc, char **argv) {
    testing::InitGoogleMock(&argc, argv);
    int result = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == std::string("--device")) {
            std::string argValue = argv[++i];
            for (auto const &enabledGpu : *productFamilyTable) {
                if (argValue == enabledGpu.second->productAbbreviation) {
                    gpu = enabledGpu.second;
                    break;
                }
            }
        }
    }

    if (gpu != nullptr) {
        result = runTests(gpu);
    } else {
        for (auto const &enabledGpu : *productFamilyTable) {
            gpu = enabledGpu.second;

            assert(testTraits[gpu->productFamily]);

            result = runTests(gpu);

            if (result != 0)
                break;
        }
    }
    return result;
}
