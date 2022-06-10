/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef int LPWSADATA;
typedef struct addrinfo ADDRINFO;

#ifdef __cplusplus
extern "C" {
#endif

int closesocket(SOCKET s) { return 0; }
int close(SOCKET s) { return 0; }
int connect(SOCKET s, const struct sockaddr *name, int namelen) { return 0; }
unsigned short htons(unsigned short hostlong) { return 0; }
unsigned long inet_addr(const char *cp) { return 0; }
int recv(SOCKET s, char *buf, int len, int flags) { return 0; }
int send(SOCKET s, const char *buf, int len, int flags) { return len; }
int shutdown(SOCKET s, int how) { return 0; }
SOCKET socket(int af, int type, int protocol) { return 0; }
struct hostent *gethostbyname(const char *name) {
    return nullptr;
}
int WSAStartup(int wVersionRequired, LPWSADATA lpWSAData) { return 0; }
int WSACleanup(void) { return 0; }
void WSASetLastError(int iError) { return; }
int WSAGetLastError(void) { return 0; }

int getaddrinfo(const char *name, const char *service, const ADDRINFO *hint, ADDRINFO **results) {
    *results = nullptr;
    return 0;
}
void freeaddrinfo(ADDRINFO *info) { return; }

#ifdef __cplusplus
}
#endif
