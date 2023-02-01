#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include "socket.h"

int socket_error() {
    return WSAGetLastError();
}

void socket_perror(const char *s) {
    int err = socket_error();

    fprintf(stderr, "%s: %d\n", s, err);
}