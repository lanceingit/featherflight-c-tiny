#pragma once

#define TRIGGER_STABLE   0
#define TRIGGER_0_TO_1   1
#define TRIGGER_1_TO_0   2

#define TRIGGER_DEF(name) static struct trigger_s name;

struct trigger_s
{
    bool last_status;
};

static inline uint8_t trigger_check(struct trigger_s* trigger, bool status)
{
    if(status != trigger->last_status) {
        trigger->last_status = status;
        if(status == true) {
            return TRIGGER_0_TO_1;
        } else {
            return TRIGGER_1_TO_0;
        }
    } else {
        trigger->last_status = status;
        return TRIGGER_STABLE;
    }
}


