/*
 * Copyright (C) 2022-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "tbx_sockets_imp.h"
#include "aub_mem_dump/settings.h"
#include <cassert>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#ifndef _WIN32_LEAN_AND_MEAN
#define _WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif
#include "ws2tcpip.h"
typedef int socklen_t;
#else
#include "memcpy_s.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define WSAECONNRESET -1
#endif
#include "tbx_proto.h"

namespace aub_stream {

TbxSocketsImp::TbxSocketsImp(std::ostream &err)
    : cerrStream(err) {
}

void TbxSocketsImp::close() {
    if (0 != m_socket) {
#ifdef _WIN32
        ::shutdown(m_socket, 0x02 /*SD_BOTH*/);

        ::closesocket(m_socket);
        ::WSACleanup();
#else
        ::shutdown(m_socket, SHUT_RDWR);
        ::close(m_socket);
#endif
        m_socket = 0;
    }
}

void TbxSocketsImp::enableThrowOnError(bool enabled) {
    throwOnError = enabled;
}

bool TbxSocketsImp::stillConnected() {
    bool success;
    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        HAS_MSG cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.hdr.msg_type = HAS_MARKER_REQ_TYPE;
        cmd.hdr.size = 0;
        cmd.hdr.trans_id = transID++;

        success = sendWriteData(&cmd, sizeof(HAS_HDR));
        if (!success) {
            break;
        }

        HAS_MSG resp;
        success = getResponseData((char *)(&resp), sizeof(HAS_HDR));
        if (!success) {
            break;
        }

        if (resp.hdr.msg_type != HAS_MARKER_RES_TYPE || cmd.hdr.trans_id != resp.hdr.trans_id) {
            success = false;
            break;
        }

        success = true;
    } while (false);

    if (!success && throwOnError) {
        throw std::runtime_error("Socket failed ping request to simulator.");
    }

    return success;
}

void TbxSocketsImp::logErrorInfo(const char *tag) {
    [[maybe_unused]] bool assertValue = false;
#ifdef _WIN32
    auto error = WSAGetLastError();
    cerrStream << tag << " TbxSocketsImp Error: <" << error << ">" << std::endl;
    if (m_socket != 0 && error == WSANOTINITIALISED) {
        assertValue = true;
    }
#else
    cerrStream << tag << " TbxSocketsImp Error: " << strerror(errno) << std::endl;
#endif
    if (!assertValue) {
        inErrorState = true;
        assert(false);
    }
}

bool TbxSocketsImp::init(const std::string &hostNameOrIp, uint16_t port, bool frontdoor) {
    do {
#ifdef _WIN32
        WSADATA wsaData;
        auto iResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR) {
            cerrStream << "Error at WSAStartup()" << std::endl;
            break;
        }
#endif

        m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) {
            logErrorInfo("Error at socket(): ");
            break;
        }

        if (!connectToServer(hostNameOrIp, port)) {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(socket_mutex);
            HAS_MSG cmd;
            memset(&cmd, 0, sizeof(cmd));
            cmd.hdr.msg_type = HAS_CONTROL_REQ_TYPE;
            cmd.hdr.size = sizeof(HAS_CONTROL_REQ);
            cmd.hdr.trans_id = transID++;

            cmd.u.control_req.time_adv_mask = 1;
            cmd.u.control_req.time_adv = 0;

            cmd.u.control_req.async_msg_mask = 1;
            cmd.u.control_req.async_msg = 0;

            cmd.u.control_req.has_mask = 1;
            cmd.u.control_req.has = 1;

            sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
        }

        if (!checkServerConfig(frontdoor)) {
            break;
        }
    } while (false);

    return m_socket != INVALID_SOCKET;
}

bool TbxSocketsImp::connectToServer(const std::string &hostNameOrIp, uint16_t port) {
    constexpr uint64_t timeoutInSeconds = 60;
    uint32_t connectionDelayInSeconds = 1;

    if (globalSettings->TbxConnectionDelayInSeconds.get() != -1) {
        connectionDelayInSeconds = static_cast<uint32_t>(globalSettings->TbxConnectionDelayInSeconds.get());
    }

    std::this_thread::sleep_for(std::chrono::seconds(connectionDelayInSeconds));

    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
    bool retryOnError = false;

    do {
        retryOnError = false;
        sockaddr_in clientService;
#if _WIN32
        struct addrinfo *result = nullptr;
        struct addrinfo hint;
        memset(&hint, 0, sizeof(hint));
        auto info_result = getaddrinfo(hostNameOrIp.c_str(), nullptr, &hint, &result);
        if (info_result) {
            cerrStream << "Host name look up failed for " << hostNameOrIp.c_str() << std::endl;
            break;
        }
        for (struct addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            if (ptr->ai_family == AF_INET) {
                struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
                memcpy_s(&clientService.sin_addr, sizeof(clientService.sin_addr), &sockaddr_ipv4->sin_addr, sizeof(sockaddr_ipv4->sin_addr));
                break;
            }
        }

        freeaddrinfo(result);
#else
        if (::isalpha(hostNameOrIp.at(0))) {
            auto hostData = ::gethostbyname(hostNameOrIp.c_str());
            if (hostData == nullptr) {
                cerrStream << "Host name look up failed for " << hostNameOrIp.c_str() << std::endl;
                break;
            }

            memcpy_s(&clientService.sin_addr, sizeof(clientService.sin_addr), hostData->h_addr, hostData->h_length);
        } else {
            clientService.sin_addr.s_addr = inet_addr(hostNameOrIp.c_str());
        }
#endif

        clientService.sin_family = AF_INET;
        clientService.sin_port = htons(port);

        if (::connect(m_socket, (SOCKADDR *)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
            auto timeDiff = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startTime).count());
            if (timeDiff < timeoutInSeconds) {
                retryOnError = true;
                continue;
            }

            logErrorInfo("Failed to connect: ");
            cerrStream << "Is TBX server process running on host system [ " << hostNameOrIp.c_str()
                       << ", port " << port << "]?" << std::endl;
            break;
        }
    } while (retryOnError);

    return !!m_socket;
}

bool TbxSocketsImp::checkServerConfig(bool frontdoor) {
    // note: we can use INNER_VAR message to check specific server configuration parameters,
    // e.g. support for frontdoor read/writes requests and *_EXT messages with 64b physical address
    if (frontdoor) {
        // read LMEMBAR value
        constexpr uint64_t LMEMBAR_DFLT = 0x000ffff000000000;
        constexpr uint64_t LMEMBAR_MASK = 0xfffffffff0000000;
        uint32_t dataLo = 0, dataHi = 0;

        readPCICFG(0, 2, 0, 0x18, &dataLo); // default BDF (0/2/0)
        readPCICFG(0, 2, 0, 0x1C, &dataHi);

        this->lmembar = ((static_cast<uint64_t>(dataHi) << 32) | dataLo) & LMEMBAR_MASK;
        if (this->lmembar == 0) {
            this->lmembar = LMEMBAR_DFLT; // default LMEMBAR
        }

        this->frontdoorMode = true;
    }

    return true;
}

bool TbxSocketsImp::readMMIO(uint32_t offset, uint32_t *data) {
    if (inErrorState) {
        return false;
    }

    bool success;

    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        HAS_MSG cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.hdr.msg_type = HAS_MMIO_REQ_TYPE;
        cmd.hdr.size = sizeof(HAS_MMIO_REQ);
        cmd.hdr.trans_id = transID++;
        cmd.u.mmio_req.offset = offset;
        cmd.u.mmio_req.data = 0;
        cmd.u.mmio_req.write = 0;
        cmd.u.mmio_req.delay = 0;
        cmd.u.mmio_req.msg_type = MSG_TYPE_MMIO;
        cmd.u.mmio_req.size = sizeof(uint32_t);

        success = sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
        if (!success) {
            break;
        }

        HAS_MSG resp;
        success = getResponseData((char *)(&resp), sizeof(HAS_HDR) + sizeof(HAS_MMIO_RES));
        if (!success) {
            break;
        }

        if (resp.hdr.msg_type != HAS_MMIO_RES_TYPE || cmd.hdr.trans_id != resp.hdr.trans_id) {
            *data = 0xdeadbeef;
            success = false;
            assert(success);
            break;
        }

        *data = resp.u.mmio_res.data;
        success = true;
    } while (false);

    return success;
}

bool TbxSocketsImp::writeMMIO(uint32_t offset, uint32_t value) {
    std::lock_guard<std::mutex> lock(socket_mutex);
    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_MMIO_REQ_TYPE;
    cmd.hdr.size = sizeof(HAS_MMIO_REQ);
    cmd.hdr.trans_id = transID++;
    cmd.u.mmio_req.msg_type = MSG_TYPE_MMIO;
    cmd.u.mmio_req.offset = offset;
    cmd.u.mmio_req.data = value;
    cmd.u.mmio_req.write = 1;
    cmd.u.mmio_req.size = sizeof(uint32_t);

    return sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
}

bool TbxSocketsImp::writePCICFG(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value) {
    std::lock_guard<std::mutex> lock(socket_mutex);
    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_PCICFG_REQ_TYPE;
    cmd.hdr.size = sizeof(HAS_PCICFG_REQ);
    cmd.hdr.trans_id = transID++;
    cmd.u.pcicfg_req.offset = offset;
    cmd.u.pcicfg_req.bus = bus;
    cmd.u.pcicfg_req.device = device;
    cmd.u.pcicfg_req.function = function;
    cmd.u.pcicfg_req.data = value;
    cmd.u.pcicfg_req.write = 1;
    cmd.u.pcicfg_req.size = sizeof(uint32_t);

    return sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
}

bool TbxSocketsImp::readPCICFG(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t *data) {
    bool success;
    do {
        HAS_MSG cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.hdr.msg_type = HAS_PCICFG_REQ_TYPE;
        cmd.hdr.size = sizeof(HAS_PCICFG_REQ);
        cmd.hdr.trans_id = transID++;
        cmd.u.pcicfg_req.write = 0;
        cmd.u.pcicfg_req.size = sizeof(uint32_t);
        cmd.u.pcicfg_req.bus = bus;
        cmd.u.pcicfg_req.device = device;
        cmd.u.pcicfg_req.function = function;
        cmd.u.pcicfg_req.offset = offset;
        std::lock_guard<std::mutex> lock(socket_mutex);
        success = sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
        if (!success) {
            break;
        }
        HAS_MSG resp;
        success = getResponseData((char *)(&resp), sizeof(HAS_HDR) + sizeof(HAS_PCICFG_RES));
        if (!success) {
            break;
        }
        if (resp.hdr.msg_type != HAS_PCICFG_RES_TYPE || cmd.hdr.trans_id != resp.hdr.trans_id) {
            *data = 0xdeadbeef;
            success = false;
            assert(success);
            break;
        }
        *data = resp.u.pcicfg_res.data;
        success = true;
    } while (false);

    return success;
}

bool TbxSocketsImp::readMemory(uint64_t addrOffset, void *data, size_t size, bool isLocalMemory) {
    if (this->frontdoorMode) {
        return readMemoryExt(addrOffset, data, size, isLocalMemory);
    }

    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_READ_DATA_REQ_TYPE;
    cmd.hdr.trans_id = transID++;
    cmd.hdr.size = sizeof(HAS_READ_DATA_REQ);
    cmd.u.read_req.address = static_cast<uint32_t>(addrOffset);
    cmd.u.read_req.address_h = static_cast<uint32_t>(addrOffset >> 32);
    cmd.u.read_req.addr_type = 0;
    cmd.u.read_req.size = static_cast<uint32_t>(size);
    cmd.u.read_req.ownership_req = 0;
    cmd.u.read_req.frontdoor = 0;
    cmd.u.read_req.cacheline_disable = cmd.u.read_req.frontdoor;
    cmd.u.read_req.memory_type = isLocalMemory ? MEM_TYPE_LOCALMEM : MEM_TYPE_SYSTEM;

    bool success;
    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        success = sendWriteData(&cmd, sizeof(HAS_HDR) + sizeof(HAS_READ_DATA_REQ));
        if (!success) {
            break;
        }

        HAS_MSG resp;
        success = getResponseData(&resp, sizeof(HAS_HDR) + sizeof(HAS_READ_DATA_RES));
        if (!success) {
            break;
        }

        if (resp.hdr.msg_type != HAS_READ_DATA_RES_TYPE || resp.hdr.trans_id != cmd.hdr.trans_id) {
            cerrStream << "Out of sequence read data packet?" << std::endl;
            success = false;
            assert(success);
            break;
        }

        success = getResponseData(data, size);
    } while (false);

    return success;
}

bool TbxSocketsImp::writeMemory(uint64_t physAddr, const void *data, size_t size, bool isLocalMemory) {
    if (this->frontdoorMode) {
        return writeMemoryExt(physAddr, data, size, isLocalMemory);
    }

    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_WRITE_DATA_REQ_TYPE;
    cmd.hdr.trans_id = transID++;
    cmd.hdr.size = sizeof(HAS_WRITE_DATA_REQ);

    cmd.u.write_req.address = static_cast<uint32_t>(physAddr);
    cmd.u.write_req.address_h = static_cast<uint32_t>(physAddr >> 32);
    cmd.u.write_req.addr_type = 0;
    cmd.u.write_req.size = static_cast<uint32_t>(size);
    cmd.u.write_req.take_ownership = 0;
    cmd.u.write_req.frontdoor = 0;
    cmd.u.write_req.cacheline_disable = cmd.u.write_req.frontdoor;
    cmd.u.write_req.memory_type = isLocalMemory ? MEM_TYPE_LOCALMEM : MEM_TYPE_SYSTEM;

    bool success;
    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        success = sendWriteData(&cmd, sizeof(HAS_HDR) + sizeof(HAS_WRITE_DATA_REQ));
        if (!success) {
            break;
        }

        success = sendWriteData(data, size);
        if (!success) {
            cerrStream << "Problem sending write data?" << std::endl;
            break;
        }
    } while (false);

    return success;
}

bool TbxSocketsImp::writeGTT(uint32_t offset, uint64_t entry) {
    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_GTT_REQ_TYPE;
    cmd.hdr.size = sizeof(HAS_GTT64_REQ);
    cmd.hdr.trans_id = transID++;
    cmd.u.gtt64_req.write = 1;
    cmd.u.gtt64_req.offset = offset / sizeof(uint64_t); // the TBX server expects GTT index here, not offset
    cmd.u.gtt64_req.data = static_cast<uint32_t>(entry & 0xffffffff);
    cmd.u.gtt64_req.data_h = static_cast<uint32_t>(entry >> 32);
    std::lock_guard<std::mutex> lock(socket_mutex);
    return sendWriteData(&cmd, sizeof(HAS_HDR) + cmd.hdr.size);
}

bool TbxSocketsImp::sendWriteData(const void *buffer, size_t sizeInBytes) {
    size_t totalSent = 0;
    auto dataBuffer = reinterpret_cast<const char *>(buffer);

    if (inErrorState) {
        return false;
    }

    do {
        auto bytesSent = ::send(m_socket, &dataBuffer[totalSent], static_cast<int>(sizeInBytes - totalSent), 0);
        if (bytesSent == 0 || bytesSent == WSAECONNRESET) {
            logErrorInfo("Connection Closed.");
            inErrorState = true;
            return false;
        }

        if (bytesSent == SOCKET_ERROR) {
            logErrorInfo("Error on send() - Entering Error State");
            inErrorState = true;
            return false;
        }
        totalSent += bytesSent;
    } while (totalSent < sizeInBytes);

    return true;
}

bool TbxSocketsImp::getResponseData(void *buffer, size_t sizeInBytes) {
    size_t totalRecv = 0;
    auto dataBuffer = reinterpret_cast<char *>(buffer);

    if (inErrorState) {
        return false;
    }

    do {
        auto bytesRecv = ::recv(m_socket, &dataBuffer[totalRecv], static_cast<int>(sizeInBytes - totalRecv), 0);
        if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
            logErrorInfo("Connection Closed.");
            inErrorState = true;
            return false;
        }

        if (bytesRecv == SOCKET_ERROR) {
            logErrorInfo("Error on recv() - Entering Error State");
            inErrorState = true;
            return false;
        }

        totalRecv += bytesRecv;
    } while (totalRecv < sizeInBytes);

    return true;
}

bool TbxSocketsImp::readMemoryExt(uint64_t addrOffset, void *data, size_t size, bool isLocalMemory) {
    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_READ_DATA_REQ_TYPE;
    cmd.hdr.trans_id = transID++;
    cmd.hdr.size = sizeof(HAS_READ_DATA_EXT_REQ);

    if (isLocalMemory) {
        addrOffset += this->lmembar;
    }

    cmd.u.read_ext_req.read_req.address = static_cast<uint32_t>(addrOffset);
    cmd.u.read_ext_req.address_h = static_cast<uint32_t>(addrOffset >> 32);
    cmd.u.read_ext_req.read_req.addr_type = 0;
    cmd.u.read_ext_req.read_req.size = static_cast<uint32_t>(size);
    cmd.u.read_ext_req.read_req.ownership_req = 0;
    cmd.u.read_ext_req.read_req.frontdoor = 1;
    cmd.u.read_ext_req.read_req.cacheline_disable = cmd.u.read_ext_req.read_req.frontdoor;
    cmd.u.read_ext_req.read_req.memory_type = MEM_TYPE_SYSTEM;

    bool success;
    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        success = sendWriteData(&cmd, sizeof(HAS_HDR) + sizeof(HAS_READ_DATA_EXT_REQ));
        if (!success) {
            break;
        }

        HAS_MSG resp;
        success = getResponseData(&resp, sizeof(HAS_HDR) + sizeof(HAS_READ_DATA_EXT_RES));
        if (!success) {
            break;
        }

        if (resp.hdr.msg_type != HAS_READ_DATA_RES_TYPE || resp.hdr.trans_id != cmd.hdr.trans_id) {
            cerrStream << "Out of sequence read data packet?" << std::endl;
            success = false;
            assert(success);
            break;
        }

        success = getResponseData(data, size);
    } while (false);

    return success;
}

bool TbxSocketsImp::writeMemoryExt(uint64_t physAddr, const void *data, size_t size, bool isLocalMemory) {
    HAS_MSG cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.hdr.msg_type = HAS_WRITE_DATA_REQ_TYPE;
    cmd.hdr.trans_id = transID++;
    cmd.hdr.size = sizeof(HAS_WRITE_DATA_EXT_REQ);

    if (isLocalMemory) {
        physAddr += this->lmembar;
    }

    cmd.u.write_ext_req.write_req.address = static_cast<uint32_t>(physAddr);
    cmd.u.write_ext_req.address_h = static_cast<uint32_t>(physAddr >> 32);
    cmd.u.write_ext_req.write_req.addr_type = 0;
    cmd.u.write_ext_req.write_req.size = static_cast<uint32_t>(size);
    cmd.u.write_ext_req.write_req.take_ownership = 0;
    cmd.u.write_ext_req.write_req.frontdoor = 1;
    cmd.u.write_ext_req.write_req.cacheline_disable = cmd.u.write_ext_req.write_req.frontdoor;
    cmd.u.write_ext_req.write_req.memory_type = MEM_TYPE_SYSTEM;

    bool success;
    do {
        std::lock_guard<std::mutex> lock(socket_mutex);
        success = sendWriteData(&cmd, sizeof(HAS_HDR) + sizeof(HAS_WRITE_DATA_EXT_REQ));
        if (!success) {
            break;
        }

        success = sendWriteData(data, size);
        if (!success) {
            cerrStream << "Problem sending write data?" << std::endl;
            break;
        }
    } while (false);

    return success;
}

TbxSockets *TbxSockets::create() {
    return new TbxSocketsImp;
}

} // namespace aub_stream
