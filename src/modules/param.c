#include "board.h"
#include <string.h>
#include "param_api.h"
#include "mtd.h"
#include "debug.h"
#include "timer.h"
#include "param.h"

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

int16_t param_get_index(char* name)
{
	for (uint8_t i = 0; i < PARAM_LIST_MAX; i++) {
		if (strcmp(name, param_list[i].name) == 0) {
            return i;
		}
	}

	return -1;    
}

int8_t param_get_val(char* name, float* val)
{
    struct param_val param;
    if(param_find(name, &param) == 0) {
        *val = *param.val;
        return 0;
    } else {
        return -1;
    }
}

int8_t param_set_val(char* name, float val)
{
    struct param_val param;
    if(param_find(name, &param) == 0) {
        *param.val = val;
        param_store_reset();
        return 0;
    } else {
        return -1;
    }    
}

void param_store_load(void)
{
    float val;
    
    mtd_seek_read(&this->mtd, 6);
        
    for(uint16_t i=0; i<PARAM_LIST_MAX; i++) {
        if(mtd_read(&this->mtd, (uint8_t*)&val, 4) >= 0) {
            if(i==1) {
                PRINT("read val(%f):%02x %02x %02x %02x \n", val, ((uint8_t*)&val)[0], ((uint8_t*)&val)[1], ((uint8_t*)&val)[2], ((uint8_t*)&val)[3]);
            }
            *param_list[i].val = val;
        }
    }
}

void param_store_reset(void)
{
    float val;
    uint32_t magic = PARAM_MAGIC;
    uint16_t total = PARAM_LIST_MAX;
    
    mtd_seek_write(&this->mtd, 0);
    
    mtd_write(&this->mtd, (uint8_t*)&magic, 4);
    mtd_write(&this->mtd, (uint8_t*)&total, 2);
    
    for(uint16_t i=0; i<PARAM_LIST_MAX; i++) {
        val = *param_list[i].val;
        //PRINT("%s->%f\n", param_list[i].name, *param_list[i].val);
        if(i==1) {
            PRINT("write val(%f):%02x %02x %02x %02x \n", val, ((uint8_t*)&val)[0], ((uint8_t*)&val)[1], ((uint8_t*)&val)[2], ((uint8_t*)&val)[3]);
        }
        mtd_write(&this->mtd, (uint8_t*)&val, 4);
    }
    mtd_sync_sector(&this->mtd, 1);    
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
   
//    param_store_reset();
    uint8_t head[6]={0};
    
    mtd_seek_read(&this->mtd, 0);
    if(mtd_read(&this->mtd, head, 6) > 0) {
        PRINT_BUF("head:", head, 6);
        
        if((uint32_t)*((uint32_t*)head) != PARAM_MAGIC || (uint16_t)*((uint16_t*)&head[4]) != PARAM_LIST_MAX) {      
            PRINT("param broken, reset!\n");
            param_store_reset();
        } else {
            PRINT("param load!\n");
            param_store_load();
        }
    }
}


