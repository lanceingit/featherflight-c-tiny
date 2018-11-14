#pragma once

typedef float (*param_get_func)(void);
typedef void (*param_set_func)(float);


struct param_val {
	char name[40];
	param_get_func get;
	param_set_func set;
};

#include "param_api.h"

struct param_s {
	char* name;
	float val;
};


#define PARAM_GROUP   _local_param
#define PARAM_TYPE    _params_local_s

#define PARAM_GROUP_START \
    static struct PARAM_TYPE {  

#define PARAM_ADD(NAME) \
    float _##NAME;

#define PARAM_GROUP_STOP \
    } PARAM_GROUP;


#define PARAM_GET(VAL) PARAM_GROUP._##VAL

#define PARAM_REGISTER(a) param_register_##a(&PARAM_GROUP);

int8_t param_get_val(char* name, float* val);
int8_t param_set_val(char* name, float val);



