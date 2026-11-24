/*
 * Copyright (C) 2022-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <cctype>

typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef int LPWSADATA;
typedef struct addrinfo ADDRINFO;

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT int closesocket(SOCKET s) { return 0; }
EXPORT int close(SOCKET s) { return 0; }
EXPORT int connect(SOCKET s, const struct sockaddr *name, int namelen) { return 0; }
EXPORT unsigned short htons(unsigned short hostlong) { return 0; }
EXPORT unsigned long inet_addr(const char *cp) { return 0; }
EXPORT int recv(SOCKET s, char *buf, int len, int flags) { return 0; }
EXPORT int send(SOCKET s, const char *buf, int len, int flags) { return len; }
EXPORT int shutdown(SOCKET s, int how) { return 0; }
EXPORT SOCKET socket(int af, int type, int protocol) { return 0; }

static int setsockoptCallCount = 0;
static int setsockoptLastLevel = 0;
static int setsockoptLastOptionName = 0;
static int setsockoptLastOptionValue = 0;
static int setsockoptReturnValue = 0;

EXPORT int setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen) {
    setsockoptCallCount++;
    setsockoptLastLevel = level;
    setsockoptLastOptionName = optname;
    setsockoptLastOptionValue = *reinterpret_cast<const int *>(optval);
    return setsockoptReturnValue;
}
EXPORT void socketStubResetSetsockopt(int returnValue) {
    setsockoptCallCount = 0;
    setsockoptLastLevel = 0;
    setsockoptLastOptionName = 0;
    setsockoptLastOptionValue = 0;
    setsockoptReturnValue = returnValue;
}
EXPORT int socketStubGetSetsockoptCallCount() { return setsockoptCallCount; }
EXPORT int socketStubGetSetsockoptLastLevel() { return setsockoptLastLevel; }
EXPORT int socketStubGetSetsockoptLastOptionName() { return setsockoptLastOptionName; }
EXPORT int socketStubGetSetsockoptLastOptionValue() { return setsockoptLastOptionValue; }
EXPORT struct hostent *gethostbyname(const char *name) {
    return nullptr;
}
EXPORT int WSAStartup(int wVersionRequired, LPWSADATA lpWSAData) { return 0; }
EXPORT int WSACleanup(void) { return 0; }
EXPORT void WSASetLastError(int iError) { return; }
EXPORT int WSAGetLastError(void) { return 0; }

EXPORT int getaddrinfo(const char *name, const char *service, const ADDRINFO *hint, ADDRINFO **results) {
    *results = nullptr;
    if (::isalpha(name[0])) {
        return 1;
    }
    return 0;
}
EXPORT void freeaddrinfo(ADDRINFO *info) { return; }

#ifdef __cplusplus
}
#endif
