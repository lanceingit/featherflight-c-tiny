#pragma once

#include <string.h>
#include <stdlib.h>

typedef void (*shell_func)(int argc, char *argv[]);      
     
void cli_init(void);     
void cli_updata(void);
void cli_regist(const char* name, shell_func cmd);
void cli_device_write(const char *format, ...);

