
#ifndef __KBOOT_H__
#define __KBOOT_H__

#include <stdarg.h>
#include "serial.h"
#include "ctype.h"

int printk(const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
void puts(const char *string);

#endif

