#pragma once

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>

#define closesocket close
typedef int SOCKET;
static constexpr int INVALID_SOCKET = -1;
static constexpr int SOCKET_ERROR = -1;

#define WSAETIMEDOUT ETIMEDOUT

#endif

int socket_error();
void socket_perror(const char *msg);