#pragma once

#include "timer.h"

typedef void(*task_callback_func)(void);

#define TASK_NAME_MAX   10

typedef struct {
    char name[TASK_NAME_MAX];
    times_t rate;
    times_t time_use;
    times_t last_run;
    task_callback_func callback;
    bool run;
} Task;

void task_init(void);
Task* task_create(char* name, times_t interval, task_callback_func cb);
void task_set_rate(Task* t, times_t time);
void task_disable(Task* t);

void scheduler_run(void);


