#include "xtask.h"
#include "xutil.h"
#include "xepoll.h"
#include "xlog.h"
#include "xsock.h"
#include "xstring.h"

#include <stdio.h>

void clientMain(void *arg) {
    int cfd = *((int*)arg);
    char buf[256];
    xstring_t req = xstring.empty();
    for (;;) {
        int res = xsock.read(cfd, buf, 128);
        if (res < 0) {
            XLOG_ERROR("read error %d, errno: %s", cfd, strerror(errno));
            goto out;
        }
        if (res == 0) {
            break;
        }
        req = xstring.catlen(req, buf, res);
        if (xstring.search(req, "\r\n\r\n") != XLIB_ERR) break;
    }
    xstring_t rsp = xstring.new("HTTP/1.1 220 OK\r\n\r\nOK\n");
    xsock.write(cfd, rsp, xstring.len(rsp));
    xstring.free(rsp);
out:
    xsock.close(cfd);
    xstring.free(req);
}

void foo(void *arg) {
    int sfd = *((int*)arg);
    for (;;) {
        int cfd = xsock.accept(sfd);
        if (cfd < 0) {
            XLOG_ERROR("xsock accept error");
            continue;
        }
        xtask_t* clientTask = xtask.create(clientMain, &cfd, 4096);
        xtask.ready(clientTask);
    }
    xsock.close(sfd);
}

int main(void) {
    int sfd = xsock.tcpServer("0.0.0.0", 7879);
    if (sfd < 0) {
        XLOG_ERROR("create tcp server error");
        return 1;
    }

    xepoll.init(10240);
    xtask_t* task1 = xtask.create(foo, &sfd, 8192);
    xtask.ready(task1);

    for (;;) {
        xepoll.process(100);
        xutil.updateTime();
        xtask.process();
    }

    return 0;
}
