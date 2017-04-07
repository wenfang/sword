#ifndef __XSIGNAL_H
#define __XSIGNAL_H


typedef struct {
    void    (*init)(void);
    void    (*register)(int sig, void(*)(int));
    void    (*process)(void);
} xsignal_p;

extern const xsignal_p xsignal;

#endif
