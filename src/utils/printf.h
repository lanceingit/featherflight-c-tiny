#pragma once

#include <stdarg.h>

int eprintf(const char * fmt, ...);
int evprintf(const char* fmt, va_list ap);
