#ifndef __XSOCK_H 
#define __XSOCK_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef struct {
    int     (*tcpServer)(const char* addr, int port);
    int     (*udpServer)(const char* addr, int port);
    int     (*tcpSocket)(void);
    int     (*udpSocket)(void);
    int     (*connect)(int fd, const char* addr, const char* port);
    int     (*accept)(int sfd);
    int     (*read)(int fd, void* buf, size_t count);
    int     (*write)(int fd, const void* buf, size_t count);
    int     (*close)(int fd);
} xsock_p;

extern xsock_p xsock;

#endif
