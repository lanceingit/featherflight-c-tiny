#pragma once

#define TRIGGER_STABLE   0
#define TRIGGER_0_TO_1   1
#define TRIGGER_1_TO_0   2

#define TRIGGER_DEF(name) static struct trigger_s name;

typedef struct
{
    bool last_status;
} trigger_s;

static inline uint8_t trigger_check(trigger_s* self, bool status)
{
    if(status != self->last_status) {
        self->last_status = status;
        if(status == true) {
            return TRIGGER_0_TO_1;
        } else {
            return TRIGGER_1_TO_0;
        }
    } else {
        self->last_status = status;
        return TRIGGER_STABLE;
    }
}


