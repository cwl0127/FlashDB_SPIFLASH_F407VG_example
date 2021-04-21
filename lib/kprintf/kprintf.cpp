#include "kprintf.h"
#include <WSerial.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

void kprintf(const char *fmt, ...)
{
    va_list args;
    size_t length;
    static char logBuf[CONSOLEBUF_SIZE];

#ifdef USE_FREERTOS
    taskENTER_CRITICAL();
#endif
    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the logBuf, we have to adjust the output
     * length. */
    length = vsnprintf(logBuf, sizeof(logBuf) - 1, fmt, args);
    if (length > CONSOLEBUF_SIZE - 1)
        length = CONSOLEBUF_SIZE - 1;
    Serial.write(logBuf);
    va_end(args);
#ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
#endif
}

