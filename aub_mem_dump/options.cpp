/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/aub_stream.h"
#include "aub_mem_dump/options.h"
#include <ostream>
#include <iostream>

struct Buffer : public std::streambuf {
    int overflow(int c) override {
        return c;
    }
};
static Buffer logBuffer;
static std::ostream logNull(&logBuffer);

namespace aub_stream {
std::ostream &log = logNull;
std::string tbxServerIp = "127.0.0.1";
uint16_t tbxServerPort = 4321;
bool tbxFrontdoorMode = false;
MMIOList MMIOListInjected;
uint32_t aubStreamCaller = 0xFFFFFFFF;

extern "C" {
void injectMMIOList(MMIOList mmioList) {
    MMIOListInjected = mmioList;
    MMIOListInjected.shrink_to_fit();
}

void setTbxServerPort(uint16_t port) {
    tbxServerPort = port;
}

void setTbxServerIp(std::string server) {
    tbxServerIp = server;
    tbxServerIp.shrink_to_fit();
}

void setTbxFrontdoorMode(bool frontdoor) {
    tbxFrontdoorMode = frontdoor;
}

void setAubStreamCaller(uint32_t caller) {
    aubStreamCaller = caller;
}
}
} // namespace aub_stream
