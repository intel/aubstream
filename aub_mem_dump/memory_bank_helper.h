/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

namespace aub_stream {

class MemoryBankHelper {
  public:
    const uint32_t memoryBanks;
    const uint64_t initialGfxAddress = 0;
    const size_t size = 0;
    size_t colorSize = 1;
    uint32_t numberOfBanks = 0;
    std::vector<uint32_t> singleBanks;

    MemoryBankHelper(uint32_t memoryBanksIn, uint64_t initialGfxAddress, size_t size) : memoryBanks(memoryBanksIn), initialGfxAddress(initialGfxAddress), size(size) {
        uint32_t mask = 1;
        do {
            if (mask & memoryBanks) {
                singleBanks.push_back(mask);
            }
            mask <<= 1;
        } while (mask <= memoryBanks);

        numberOfBanks = uint32_t(singleBanks.size());
        // split memory evenly to banks
        if (numberOfBanks > 0) {
            colorSize = size / numberOfBanks;
        }
    }
    uint32_t getMemoryBank(uint64_t gfxAddress) {
        if (numberOfBanks == 0) {
            return 0;
        } else if (numberOfBanks == 1) {
            return singleBanks[0];
        }

        assert(gfxAddress >= initialGfxAddress);

        uint64_t diff = gfxAddress - initialGfxAddress;
        auto bank = singleBanks[(diff / colorSize) % numberOfBanks];
        return bank;
    }
};

} // namespace aub_stream
