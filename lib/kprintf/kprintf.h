#ifndef __KPRINTF_H
#define __KPRINTF_H

#define CONSOLEBUF_SIZE 128

#ifdef __cplusplus
extern "C" {
#endif

void kprintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif