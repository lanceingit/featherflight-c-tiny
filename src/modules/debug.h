#pragma once

#include <stdio.h>

#include "cli.h"


typedef enum
{
    DEBUG_LEVEL_ERR = 0,
    DEBUG_LEVEL_WARN,
    DEBUG_LEVEL_INFO,
    DEBUG_LEVEL_DEBUG,
} debug_level_e;

typedef enum  
{
    DEBUG_ID_MIN = 0,
	DEBUG_ID_HMC5883,
	DEBUG_ID_I2C,
	DEBUG_ID_MPU6050,
	DEBUG_ID_MS5611,
	DEBUG_ID_SERIAL,
	DEBUG_ID_SPI,
	DEBUG_ID_SPI_FLASH,
	DEBUG_ID_TIMER,
    DEBUG_ID_MOTOR,    
    DEBUG_ID_MAVLINK,
    DEBUG_ID_WWLINK,
    DEBUG_ID_EST,
    DEBUG_ID_MIXER,
    DEBUG_ID_CMD,
    DEBUG_ID_ATTC,
	DEBUG_ID_NAV,
    DEBUG_ID_SENS,
    DEBUG_ID_ALTC,
    DEBUG_ID_LOG,
    DEBUG_ID_MTD,
    DEBUG_ID_PARAM,
    DEBUG_ID_CLI,
    DEBUG_ID_SCHEDULER,
    DEBUG_ID_DEBUG,
    DEBUG_ID_FIFO,
    DEBUG_ID_PERF,
    DEBUG_ID_LPF,
    DEBUG_ID_PID,
    DEBUG_ID_MM,
    DEBUG_ID_LIST,
    DEBUG_ID_MATH,
    DEBUG_ID_MATRIX,
    DEBUG_ID_Q,
    DEBUG_ID_VECTOR,
    DEBUG_ID_DCM,
    DEBUG_ID_ATT_CF,
    DEBUG_ID_ATT_Q,
    DEBUG_ID_STAB,
    DEBUG_ID_ALTHOLD,
    DEBUG_ID_LAND,
	DEBUG_ID_MAX,	
} debug_id_e;

extern debug_level_e debug_level;
extern debug_id_e debug_module;
extern char* debug_module_list[DEBUG_ID_MAX];

#ifdef LINUX        
    #define PRINT(format,...)\
        do {\
            printf(""format"",##__VA_ARGS__ );\
            cli_device_write(""format"",##__VA_ARGS__ );\
        } while (0)
#else
    #define PRINT(format,...)\
        do {\
            cli_device_write(""format"",##__VA_ARGS__ );\
        } while (0)
#endif

#define PRINT_BUF(str, buf, len)\
    do {\
        PRINT("%s",str);\
        for(uint8_t i=0; i<len; i++) { \
            PRINT("%02x ", buf[i]);  \
        } \
        PRINT("\n"); \
    } while(0)
    
#define DEBUG(module,format,...)\
	do {\
		if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){\
			cli_device_write("[DEBUG][%s]"format, debug_module_list[module], ##__VA_ARGS__ );\
		}\
	} while (0)

#define DEBUG_PRINT(format,...)\
	do {\
        cli_device_write(""format"", ##__VA_ARGS__ );\
	} while (0)
    
#define DEBUG_BUF(module, str, buf, len)\
   do {\
       if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){\
           DEBUG_PRINT("[DEBUG][%s]%s",debug_module_list[module], str);\
           for(uint8_t i=0; i<len; i++) { \
                   DEBUG_PRINT("%02x ", buf[i]);  \
           } \
           DEBUG_PRINT("\n"); \
       } \
   } while(0)
   
#define DEBUG_BUF_DEC(module, str, buf, len)\
   do {\
       if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){\
           DEBUG_PRINT("[DEBUG][%s]%s",debug_module_list[module], str);\
           for(uint8_t i=0; i<len; i++) { \
                   DEBUG_PRINT("%d ", buf[i]);  \
           } \
           DEBUG_PRINT("\n"); \
       } \
   } while(0)   

#define INFO(module,format,...)\
	do {\
		if(debug_level >= DEBUG_LEVEL_INFO){ \
            cli_device_write("[INFO][%s]"format, debug_module_list[module], ##__VA_ARGS__);\
		} \
	} while (0)

#define WARN(module,format,...)\
	do {\
		if(debug_level >= DEBUG_LEVEL_WARN){ \
            cli_device_write("[WARN][%s]"format, debug_module_list[module], ##__VA_ARGS__);\
		} \
	} while (0)
    
#define ERR(module,format,...)\
	do {\
		cli_device_write("[ERR][%s]"format,  debug_module_list[module], ##__VA_ARGS__);\
	} while (0)
    
    
//void DEBUG(debug_id_e module, char* format,...);
//void DEBUG_PRINT(char* format,...);
//void DEBUG_BUF(debug_id_e module,  char* str, uint8_t* buf, uint8_t len);
//void DEBUG_BUF_DEC(debug_id_e module,  char* str, uint8_t* buf, uint8_t len);
//void INFO(debug_id_e module, char* format,...);
//void WARN(debug_id_e module, char* format,...);
//void ERR(debug_id_e module, char* format, ...);    
    
void debug_init(void);       

