#pragma once

typedef struct
{
    float volt_raw;
    float volt_raw_last;
    float volt;
} Batt;

extern Batt batt;

void battery_init(void);
void battery_update(void);
