#pragma once

struct param_val {
    char name[40];
    float* val;
};

#include "param_api.h"

struct param_s {
    char* name;
    float val;
};


#define PARAM_GROUP   _global_param
#define PARAM_TYPE    _params_global_s


#define PARAM_GET(VAL) PARAM_GROUP._##VAL
#define PARAM_POINT(VAL) &PARAM_GROUP._##VAL

int8_t param_get_val(char* name, float* val);
int8_t param_set_val(char* name, float val);

void param_init(void);
int8_t param_find(char* name, struct param_val* param);
int16_t param_get_index(char* name);
void param_store_reset(void);

