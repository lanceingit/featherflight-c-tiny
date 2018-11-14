#pragma once

#include <stdlib.h>

void mm_init(void);
void* mm_malloc(uint32_t s);
void mm_free(void* m);
void mm_print_info(void);



