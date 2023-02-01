#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

int socket_error();
void socket_perror(const char *msg);
#endif
