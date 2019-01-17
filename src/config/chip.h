#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef F3_EVO
#include "stm32f30x.h"
#elif LINUX
#elif SM701
#include "am_mcu_apollo.h"
#endif
