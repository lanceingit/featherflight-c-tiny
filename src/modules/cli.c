/**                                               _____           ,-.
 * _______       _____       _____                ___   _,.      /  /
 * ___    |__   ____(_)_____ __  /______________  __   ; \____,-==-._  )
 * __  /| |_ | / /_  /_  __ `/  __/  __ \_  ___/  _    //_    `----' {+>
 * _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /      _    `  `'--/  /-'`(
 * /_/  |_|____/ /_/  \__,_/ \__/ \____//_/       _          /  /
 *                                                           `='
 * 
 * cli.c
 *
 * v1.4
 *
 * command line interface module
 */
#include "board.h"

#include <string.h>
#include <stdarg.h>

#ifdef STM32F3
    #include "serial.h"
#elif LINUX
    #include <stdlib.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
#elif RTTHREAD
    #include <rtdevice.h>
    #include <dfs_posix.h>
#endif

#include "cli.h"
#include "debug.h"
#include "mm.h"
#include "printf.h"

/*
intput -> string ->  cmd/arg0/arg1/... -> function(arg0,arg1,...) -> output
name -> cmd  == string -> function
string -> arg == string parse
*/


struct buildin_s {
    const char* name;
    shell_func cmd;
};

#define BUFFER_SIZE 50
static char cmd_buf[BUFFER_SIZE+1];
static uint8_t read_cnt=0;
static uint16_t cmd_total;

static struct buildin_s shell[100];
static void cli_handle_cmd(char* buf);

static uint8_t read_buffer[BUFFER_SIZE+1];
static uint8_t write_buffer[500];

void help_shell(int argc, char* argv[]);
void reboot_shell(int argc, char* argv[]);


#ifdef STM32F3
    #define RX_BUF_SIZE 300    
    #define TX_BUF_SIZE 512 

    Serial* cli_port;
    uint8_t cli_rxBuf[RX_BUF_SIZE]; 
    uint8_t cli_txBuf[TX_BUF_SIZE];
#elif LINUX
    #define CLI_PORT  14558
    static int cli_socket_fd = -1;
    static struct sockaddr_in recv_addr;
    static int addr_len = 0;

#elif RTTHREAD
    static rt_device_t cli_dev = RT_NULL;
#endif


void help_shell(int argc, char* argv[])
{
    for(uint8_t i=0; i<cmd_total; i++) {
        cli_device_write("%s\n", shell[i].name);
    }
    cli_device_write("\n");
}

void reboot_shell(int argc, char* argv[])
{
#ifdef STM32F3
    NVIC_SystemReset();
#elif LINUX
#elif APOLLO
    am_hal_sysctrl_aircr_reset();
#endif
}

///////////////////////////////////////////////////////

void cli_regist(const char* name, shell_func cmd)
{
    shell[cmd_total].name = name;
    shell[cmd_total].cmd = cmd;
    cmd_total++;
}

int cli_device_read(uint8_t* data, uint16_t size)
{
#ifdef STM32F3
    uint16_t i;
    
    for(i=0; i<size; i++) {
        if(serial_read(cli_port, &data[i]) < 0)
            break;
    }    
    
    return i;    

#elif LINUX
    int len = 0;

    bzero(socket_buffer, size);
    len = recvfrom(cli_socket_fd, data, size, 0, (struct sockaddr*)&recv_addr,
                   (socklen_t*)&addr_len);

    if(len > 0) {
//        char* client_ip = inet_ntoa(recv_addr.sin_addr);
//        PRINT("ip:%s port:%d len:%d\n", client_ip, recv_addr.sin_port, len);
//        for(uint16_t i=0; i<len; i++) {
//        	PRINT("%x ", data[i]);
//        }
//        PRINT("\n");
    }
    return len;

#elif RTTHREAD
    int len;
    len = rt_device_read(cli_dev, 0, buffer, size);

    if(len > 0) {
        return len;
    }
    else {
        len = rt_strlen(protocol_buf);
        if(len > 0) {
            rt_strncpy((char*)buffer, protocol_buf, len);
            protocol_buf[0] = 0;
        }
    }
    return len;

#else
    return 0;
#endif
}

void cli_device_write(const char* format, ...)
{
    va_list args;
    int len;

    va_start(args, format);
    len = evsprintf((char*)write_buffer, format, args);
    va_end(args);

#ifdef STM32F3
    serial_write(cli_port, write_buffer, len);  
#elif LINUX
    sendto(cli_socket_fd, write_buffer, len, 0, (struct sockaddr*)&recv_addr, addr_len);

#elif RTTHEAD
    PRINT("%s", write_buffer);
#endif
//	PRINT("%s", write_buffer);
}

bool cli_char_parse(char c)
{
    if(c == '\b') {
        if(read_cnt > 0) {
            read_cnt--;
        }
    }
    else if(c == '\n' || c == '\r') {
        cmd_buf[read_cnt] = '\0';
        read_cnt = 0;
        return true;
    }
    else {
        cmd_buf[read_cnt++] = c;
    }

    if(read_cnt >=  BUFFER_SIZE) {
        read_cnt = 0;
    }

    return false;
}

static void cli_handle_cmd(char* buf)
{
    uint8_t len = strlen(buf);

    if(len==0||(len==1&&!strcmp(buf, "\n"))||(len==2&&!strcmp(buf, "\r\n"))) {
        cli_device_write("\r>");
        return;
    }

    int argc=0;
    char* argv[10];
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

    if((!strcmp(argv[0], "help")) || (!strcmp(argv[0], "?"))) {
        help_shell(argc, argv);
    }
    else {
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

void cli_update(void)
{
    int len = cli_device_read(read_buffer, BUFFER_SIZE);

    if(len<=0) return;

    for(uint16_t i=0; i<len; i++) {
        if(cli_char_parse(read_buffer[i])) {
            cli_handle_cmd(cmd_buf);
        }
    }
}

#ifdef RTTHREAD
void cli_thread_entry(void* parameter)
{
    while(1) {
        cli_updata();
        rt_thread_delay(RT_TICK_PER_SECOND/50);
    }
}
#endif

void cli_device_init(void)
{
#ifdef STM32F3
    cli_port = serial_open(CLI_UART, 115200, cli_rxBuf, RX_BUF_SIZE, cli_txBuf, TX_BUF_SIZE);
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
    flag = fcntl(cli_socket_fd, F_GETFL, 0);
    fcntl(cli_socket_fd, F_SETFL, flag | O_NONBLOCK);

    if(bind(cli_socket_fd, (struct sockaddr*)&addr, sizeof(addr))<0) {
        perror("cli bind failed\n");
        exit(1);
    }
    else {
        PRINT("cli init success\n");
    }

#elif RTTHREAD
    rt_thread_t cli_thread;

    cli_dev = rt_console_get_device();

    cli_thread = rt_thread_create("cli",
                                  cli_thread_entry, RT_NULL,
                                  2*1024, 10, 10);
    if(cli_thread != RT_NULL) {
        INFO(DEBUG_ID_CLI, "cli_thread init!\r\n");
        rt_thread_startup(cli_thread);
    }
#endif
}

void cli_print_logo(void)
{
    cli_device_write("\n");
    cli_device_write(" _______       _____       _____\n");
    cli_device_write(" ___    |__   ____(_)_____ __  /______________ \n");
    cli_device_write(" __  /| |_ | / /_  /_  __ `/  __/  __ \\_  ___/\n");
    cli_device_write(" _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /\n");\
    cli_device_write(" /_/  |_|____/ /_/  \\__,_/ \\__/ \\____//_/\n\n");    
}

void cli_init(void)
{
    cli_device_init();
    cli_regist("help", help_shell);
    cli_regist("reboot", reboot_shell);

    cli_print_logo();
}
