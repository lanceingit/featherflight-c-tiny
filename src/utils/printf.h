#pragma once

#include <stdarg.h>

int esprintf(char* buf, const char* fmt, ...);
int evsprintf(char* buf, const char* fmt, va_list ap);
