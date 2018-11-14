#pragma once

#include <stdbool.h>
#include "rotation.h"
#include "vector.h"

typedef bool (init_func)(void);
typedef void (update_func)(void);
typedef void (imu_update_func)(Vector* acc, Vector* gyro);
typedef bool (read_status_func)(void);
typedef void (set_status_func)(void);
typedef void (set_val_func)(uint8_t);

typedef bool (run_func)(float dt);
