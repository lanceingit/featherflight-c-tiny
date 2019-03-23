#pragma once

#include <stdbool.h>
#include "rotation.h"
#include "vector.h"

typedef bool (*const init_func)(void);
typedef void (*const update_func)(void);
typedef void (*const imu_update_func)(Vector* acc, Vector* gyro);
typedef bool (*const read_status_func)(void);
typedef void (*const set_status_func)(void);
typedef void (*const set_val_func)(uint8_t);

typedef bool (*const run_func)(float dt);
