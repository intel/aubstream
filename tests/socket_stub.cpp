/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

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
EXPORT struct hostent *gethostbyname(const char *name) {
    return nullptr;
}
EXPORT int WSAStartup(int wVersionRequired, LPWSADATA lpWSAData) { return 0; }
EXPORT int WSACleanup(void) { return 0; }
EXPORT void WSASetLastError(int iError) { return; }
EXPORT int WSAGetLastError(void) { return 0; }

EXPORT int getaddrinfo(const char *name, const char *service, const ADDRINFO *hint, ADDRINFO **results) {
    *results = nullptr;
    return 0;
}
EXPORT void freeaddrinfo(ADDRINFO *info) { return; }

#ifdef __cplusplus
}
#endif
