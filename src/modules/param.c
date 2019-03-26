#include "board.h"
#include <string.h>
#include "param_api.h"
#include "mtd.h"

#define PARAM_MAGIC     0xDC1E1084

typedef struct {
    Mtd mtd;
} Param;

Param param;
static Param* this = &param;

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

void param_store_load(void)
{
    float val;
    
    mtd_seek(&this->mtd, 6);
        
    for(uint16_t i=0; i<PARAM_LIST_MAX; i++) {
        if(mtd_read(&this->mtd, i*4, (uint8_t*)&val, 4) >= 0) {
            param_list[i].set(val);
        }
    }
}

void param_store_reset(void)
{
    float val;
    uint32_t magic = PARAM_MAGIC;
    uint16_t total = PARAM_LIST_MAX;
    
    mtd_seek(&this->mtd, 0);
    
    mtd_write(&this->mtd, (uint8_t*)&magic, 4);
    mtd_write(&this->mtd, (uint8_t*)&total, 2);
    
    for(uint16_t i=0; i<PARAM_LIST_MAX; i++) {
        val = param_list[i].get();
        mtd_write(&this->mtd, (uint8_t*)&val, 4);
    }
}

/*

    magic[4] 
    total[2]
    param[4] --- float
    ...
    ...

*/
void param_init(void)
{
    mtd_init(&this->mtd, 0, 1);
   
    uint8_t head[6];
    
    if(mtd_read(&this->mtd, 0, head, 6) >= 0) {
        if((uint32_t)*((uint32_t*)head) != PARAM_MAGIC || (uint16_t)*((uint16_t*)&head[4]) != PARAM_LIST_MAX) {            
            param_store_reset();
        } else {
            param_store_load();
        }
    }
}


