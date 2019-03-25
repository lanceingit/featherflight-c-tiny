#include "board.h"

#include "debug.h"
#include "debug_module_list.h"

debug_level_e debug_level = DEBUG_LEVEL_INFO;
debug_id_e debug_module = DEBUG_ID_MIN;

void debug_shell(int argc, char *argv[]);


//void DEBUG(debug_id_e module, char* format,...)
//{
//	va_list args;

//	va_start(args,format);    
//    if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){
//        cli_device_write("[DEBUG][%s]""format", debug_module_list[module], args);
//    }
//    va_end(args);
//}
//    
//void DEBUG_BUF(debug_id_e module,  char* str, uint8_t* buf, uint8_t len)
//{
//   if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){
//       cli_device_write("[DEBUG][%s]%s",debug_module_list[module], str);
//       for(uint8_t i=0; i<len; i++) { 
//               cli_device_write("%02x ", buf[i]);  
//       } 
//       cli_device_write("\n"); 
//   } 
//}
//   
//void DEBUG_BUF_DEC(debug_id_e module,  char* str, uint8_t* buf, uint8_t len)
//{
//       if(debug_module == module && debug_level >= DEBUG_LEVEL_DEBUG){
//           cli_device_write("[DEBUG][%s]%s",debug_module_list[module], str);
//           for(uint8_t i=0; i<len; i++) { 
//                   cli_device_write("%d ", buf[i]);  
//           } 
//           cli_device_write("\n"); 
//       } \
//}

//void INFO(debug_id_e module, char* format,...)
//{
//	va_list args;

//	va_start(args,format);        
//    if(debug_level >= DEBUG_LEVEL_INFO){ 
//        cli_device_write("[INFO][%s]""format""", debug_module_list[module], args);
//        log_record("[INFO][%s]%s""format", debug_module_list[module], args);
//    }
//    va_end(args);
//}

//void WARN(debug_id_e module, char* format,...)
//{
//    va_list args;

//	va_start(args,format);    
//    if(debug_level >= DEBUG_LEVEL_WARN){ 
//            cli_device_write("[WARN][%s]""format", debug_module_list[module], args);
//            log_record("[WARN][%s""format", debug_module_list[module], args);
//	}
//    va_end(args);
//}

//void ERR(debug_id_e module, char* format, ...)
//{
//	va_list args;

//	va_start(args,format);        
//    cli_device_write("[ERR][%s]""format", debug_module_list[module], args);
//    log_record("[ERR][%s]""format", debug_module_list[module], args);
//    va_end(args);
//}


void debug_list_modules(void)
{
	uint16_t count;

	cli_device_write("modules:\r\n");
	for(count = 0 ; count < DEBUG_ID_MAX ; count++){
		cli_device_write("[%d][%s]\r\n", count, debug_module_list[count]);
	}
}

void debug_status_show(void)
{
	cli_device_write("debug level:%d module:%d name:%s\r\n",debug_level, debug_module, debug_module_list[debug_module]);
}

void debug_level_set(debug_level_e level)
{
	debug_level = level;
}

bool debug_module_set(char * module)
{
	uint8_t count;

	debug_level_set(DEBUG_LEVEL_DEBUG);
	for(count = 0 ; count < DEBUG_ID_MAX ; count++){
		if(strcasecmp(debug_module_list[count], module) == 0){
			debug_module = (debug_id_e)count;
			cli_device_write("debug module:[%d][%s]\r\n",debug_module, debug_module_list[debug_module]);
			return true;
		}
	}
	return false;
}

void debug_init(void)
{
    cli_regist("debug", debug_shell);
    
//	debug_level = DEBUG_LEVEL_INFO;
	debug_level = DEBUG_LEVEL_DEBUG;
	debug_module = DEBUG_ID_MIN;    
}

void debug_shell(int argc, char *argv[])
{
	if(argc == 2) {
		if(strcmp(argv[1],"list") == 0) {
			debug_list_modules();
			return;
		}
		else if(strcmp(argv[1],"status") == 0) {
			debug_status_show();
			return;
		}
		else if(strcmp(argv[1],"all") == 0) {
			debug_level_set(DEBUG_LEVEL_DEBUG);
			return;
		} else if(strcmp(argv[1],"off") == 0) {
			debug_module = DEBUG_ID_MIN;
			debug_level_set(DEBUG_LEVEL_INFO);        
			return;
		} else {
			if(debug_module_set(argv[1])) {
				return;
			}
        }
	} else if(argc == 3) {
		if(strcmp(argv[1],"level") == 0) {
			debug_level_set((debug_level_e)atoi(argv[2]));
            return;
		}
	}

	cli_device_write("missing command: try 'list', 'status', 'all', 'off', 'level n', 'module name'");
}
