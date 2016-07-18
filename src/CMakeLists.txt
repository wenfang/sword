set(SRC_LIST xstring.c xlog.c xmem.c xsock.c xtask.c xepoll.c xutil.c main.c)
add_definitions(-std=gnu99)
add_executable(../bin/sword ${SRC_LIST})
