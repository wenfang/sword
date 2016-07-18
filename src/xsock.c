#include "xsock.h"
#include "xcommon.h"
#include "xtask.h"
#include "xepoll.h"
#include "xlog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#define DEFAULT_BACKLOG	10240

static int _close(int fd);

static bool _addrValid(const char* addr) {
    if (!addr) return false;
    int i, dot_cnt = 0, addrLen = strlen(addr);
    for (i=0; i<addrLen; i++) {
        if (addr[i] >='0' && addr[i]<='9') continue;
        if (addr[i] == '.') {
            dot_cnt++;
            continue;
        }
        return false;
    }
    if (dot_cnt != 3) return false;
    return true;
}

static bool _setNonBlock(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0) return false;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) return false;
    return true;
}

static int _tcpServer(const char* addr, int port) {
    int sfd;
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return XLIB_ERR;
    //set socket option
    int flags = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags)) < 0) goto error_out;
    if (setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags)) < 0) goto error_out;
    //flags = 5;
    //if (setsockopt(sfd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &flags, sizeof(flags)) < 0) goto error_out;
    // set linger
    struct linger ling = {0, 0};
    if (setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling)) < 0) goto error_out;
    // 设置非阻塞
    if (!_setNonBlock(sfd)) goto error_out;
    //set socket address
    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    if (_addrValid(addr)) {
        if (inet_aton(addr, &saddr.sin_addr) == 0) goto error_out;
    } else {
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    saddr.sin_port = htons(port);
    //bind and listen
    if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) goto error_out;
    if (listen(sfd, DEFAULT_BACKLOG) != 0) goto error_out;
    return sfd;

error_out:
    _close(sfd);
    return XLIB_ERR;
}

static int _udpServer(const char* addr, int port) {
    int sfd;
    if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) return XLIB_ERR;

    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    if (_addrValid(addr)) {
        if (inet_aton(addr, &saddr.sin_addr) == 0) goto error_out;
    } else {
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    saddr.sin_port = htons(port);

    if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) goto error_out;
    return sfd;

error_out:
    _close(sfd);
    return XLIB_ERR;
}

static int _accept(int sfd) {
    struct sockaddr_in caddr;
    socklen_t caddr_len = 0;
    bzero(&caddr, sizeof(caddr));

    int cfd; 
    for (;;) { 
        cfd = accept(sfd, (struct sockaddr*)&caddr, &caddr_len);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                xepoll.setRead(sfd, xtask.current());
                xtask.schedule();
                continue;
            }
        } else {
            _setNonBlock(cfd);
        }
        break;
    }
    xepoll.unsetRead(sfd);
    return cfd;
}

static int _read(int fd, void* buf, size_t count) {
    int res;
    for (;;) {
        res = read(fd, buf, count);
        if (res < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                xepoll.setRead(fd, xtask.current());
                xtask.schedule();
                continue;
            }
        }
        break;
    }
    xepoll.unsetRead(fd);
    return res; 
}

static int _write(int fd, const void* buf, size_t count) {
    int res;
    for (;;) {
        res = write(fd, buf, count);
        if (res < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                xepoll.setWrite(fd, xtask.current());
                xtask.schedule();
                continue;
            }
        }
        break;
    }
    xepoll.unsetWrite(fd);
    return res;
}

static int _tcpSocket(void) {
    return socket(AF_INET, SOCK_STREAM, 0);
}

static int _udpSocket(void) {
    return socket(AF_INET, SOCK_DGRAM, 0);
}

static int _close(int fd) {
    return close(fd);
}


xsock_p xsock = {
    _tcpServer,
    _udpServer,
    _tcpSocket,
    _udpSocket,
    _accept,
    _read,
    _write,
    _close
};
