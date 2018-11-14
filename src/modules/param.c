#include "board.h"
#include <string.h>
#include "param_api.h"

int8_t param_find(char* name, struct param_val* param)
{
	for (uint8_t i = 0; i < PARAM_LIST_MAX; i++) {
		if (strcmp(name, param_list[i].name) == 0) {
			*param = param_list[i];
            return 0;
		}
	}

	return -1;    
}

int8_t param_get_val(char* name, float* val)
{
	// for (uint8_t i = 0; i < PARAM_LIST_MAX; i++) {
	// 	if (strcmp(name, param_list[i].name) == 0) {
	// 		*val = param_list[i].get();
    //         return 0;
	// 	}
	// }

	// return -1;

    struct param_val* param=NULL;
    if(param_find(name, param) == 0) {
        *val = param->get();
        return 0;
    } else {
        return -1;
    }
}

int8_t param_set_val(char* name, float val)
{
	// for (uint8_t i = 0; i < PARAM_LIST_MAX; i++) {
	// 	if (strcmp(name, param_list[i].name) == 0) {
	// 		param_list[i].set(val);
    //         return 0;
	// 	}
	// }

	// return -1;

    struct param_val* param=NULL;
    if(param_find(name, param) == 0) {
        param->set(val);
        return 0;
    } else {
        return -1;
    }    
}
