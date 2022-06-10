/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "memory_banks.h"
#include <sstream>

namespace aub_stream {

std::string memoryBanksToString(uint32_t memoryBanks, const std::string &separator) {
    std::ostringstream str;

    if (memoryBanks) {
        int i = 0;
        while (memoryBanks) {
            if (memoryBanks & 1) {
                str << separator << "bank" << i;
            }
            memoryBanks >>= 1;
            ++i;
        }
    } else {
        str << separator << "sys";
    }

    return str.str();
}

} // namespace aub_stream
