#include "board.h"

#include <string.h>
#include <stdarg.h>

#ifdef LINUX
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include "cli.h"
#include "debug.h"
#include "mm.h"

/*
intput -> string ->  cmd/arg0/arg1/... -> function(arg0,arg1,...) -> output

name -> cmd  == string -> function
string -> arg == string parse

*/



struct buildin_s
{
    const char* name;
    shell_func cmd;
};

#define BUFFER_SIZE 50 
static char cmd_buf[BUFFER_SIZE+1];
static uint8_t read_cnt=0;
static uint16_t cmd_total;

static struct buildin_s shell[100];
static void cli_handle_cmd(char* buf);

void help_shell(int argc, char *argv[]);
void reboot_shell(int argc, char *argv[]);

static uint8_t read_buffer[BUFFER_SIZE+1];
static uint8_t write_buffer[500];

#ifdef LINUX
#define CLI_PORT  14558
static int cli_socket_fd = -1;
static struct sockaddr_in recv_addr;
static int addr_len = 0;
#endif

void cli_device_init(void)
{
#ifdef F3_EVO
#elif LINUX     
	int flag = 0;
	struct sockaddr_in addr;

	if((cli_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cli socket failed\n");
		exit(1);
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(CLI_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	flag = fcntl(cli_socket_fd , F_GETFL , 0);
	fcntl(cli_socket_fd,F_SETFL,flag | O_NONBLOCK);

	if(bind(cli_socket_fd, (struct sockaddr *)&addr, sizeof(addr))<0){
		perror("cli bind failed\n");
		exit(1);
	} else {
		PRINT("cli init success\n");
	}
#endif    
}

void cli_init(void)
{
	cli_device_init();
    cli_regist("help", help_shell);
    cli_regist("reboot", reboot_shell);
}

int cli_device_read(uint8_t* socket_buffer, uint16_t size)
{
#ifdef F3_EVO
    return 0;    
#elif LINUX     
    int len = 0;

    bzero(socket_buffer, size);
    len = recvfrom(cli_socket_fd, socket_buffer, size, 0, (struct sockaddr *)&recv_addr, (socklen_t*)&addr_len);

    if(len > 0) {
//        char* client_ip = inet_ntoa(recv_addr.sin_addr);
//        PRINT("ip:%s port:%d len:%d\n", client_ip, recv_addr.sin_port, len);
//        for(uint16_t i=0; i<len; i++) {
//        	PRINT("%x ", socket_buffer[i]);
//        }
//        PRINT("\n");
    }
    return len;
#else
    return 0;    
#endif    
}

void cli_device_write(const char *format, ...)
{
	va_list args;
	int len;

	va_start(args,format);
//	len = vsprintf((char*)write_buffer, format, args);
	va_end(args);

#ifdef F3_EVO
#elif LINUX    
	sendto(cli_socket_fd, write_buffer, len, 0, (struct sockaddr *)&recv_addr, addr_len);
#endif    
//	PRINT("%s", write_buffer);
}

bool cli_char_parse(char c)
{
	if(c == '\b') {
		if(read_cnt > 0) {
			read_cnt--;
		}
	} else if(c == '\n' || c == '\r') {
		cmd_buf[read_cnt] = '\0';
		read_cnt = 0;
		return true;
	} else {
		cmd_buf[read_cnt++] = c;
	}

    if(read_cnt >=  BUFFER_SIZE) {   
        read_cnt = 0;
    }

    return false;
}

void cli_updata(void)
{
    int len = cli_device_read(read_buffer, BUFFER_SIZE);

    if(len<=0) return;

    for(uint16_t i=0; i<len; i++) {
        if(cli_char_parse(read_buffer[i])) {
        	cli_handle_cmd(cmd_buf);
        }
    }

}

static void cli_handle_cmd(char* buf)
{
    uint8_t len = strlen(buf);
    
    if(len==0||(len==1&&!strcmp(buf, "\n"))||(len==2&&!strcmp(buf, "\r\n"))) {
        cli_device_write("\r>");
        return;
    }
    
    int argc=0;
    char *argv[10];
    char* p = buf;
    uint8_t argv_len=0;
    
    while(*p != '\0') {
		while(*p != ' ' && *p != '\0') {
            p++;
            argv_len++;
		}
        
        argv[argc] = mm_malloc(argv_len+1);
        strncpy(argv[argc], p-argv_len, argv_len);      
        argv[argc][argv_len] = '\0';       
        argc++;
        argv_len = 0;
        
		while(*p == ' ' && *p != '\0') {
            p++;
		}         
    }
    
//    for(uint8_t i=0; i<argc;i++)
//    {
//        PRINT("argv%d %s\r\n", i, argv[i]);
//    }
        
    if ((!strcmp(argv[0], "help")) || (!strcmp(argv[0], "?"))) {
        help_shell(argc, argv);
    } else {
        bool cmd_find= false;
        for(uint8_t i=0; i<cmd_total; i++) {
            if(!strcmp(argv[0], shell[i].name)) {
                shell[i].cmd(argc, argv);
                cmd_find = true;
                break;
            }
        }
        
        if(!cmd_find) {
            cli_device_write("%s: command not found\n\n", argv[0]);
        }
    }
    
    while(argc--) {
        mm_free(argv[argc]);
    }

    cli_device_write("\n>");
}

void help_shell(int argc, char *argv[])
{
    for(uint8_t i=0; i<cmd_total; i++) {
        cli_device_write("%s\n", shell[i].name);
    }
    cli_device_write("\n");
}

void reboot_shell(int argc, char *argv[])
{
#ifdef F3_EVO
    NVIC_SystemReset();
#elif LINUX    
#endif    
}

void cli_regist(const char* name, shell_func cmd)
{
    shell[cmd_total].name = name;
    shell[cmd_total].cmd = cmd;
    cmd_total++;
}


