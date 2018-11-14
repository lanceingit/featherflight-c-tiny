#pragma once

#include <stdio.h>

#include "cli.h"

#define PRINT(format,...)\
	do {\
        printf(""format"",##__VA_ARGS__ );\
		cli_device_write(""format"",##__VA_ARGS__ );\
	} while (0)

#define PRINT_BUF(str, buf, len)\
    do {\
        PRINT("%s",str);\
        for(uint8_t i=0; i<len; i++) { \
            PRINT("%02x ", buf[i]);  \
        } \
        PRINT("\n"); \
    } while(0)

