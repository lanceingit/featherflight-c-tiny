#include "board.h"
#include "scheduler.h"
#include "debug.h"
#include "mm.h"
#include <string.h>

#define TASK_MAX     20

static struct task_s* task_tab[TASK_MAX] = {NULL};
static uint8_t task_cnt=0;

void task_shell(int argc, char *argv[]);

void task_init(void)
{
    cli_regist("task", task_shell);
}

struct task_s* task_create(char* name, times_t interval, task_callback_func cb)
{
    if(task_cnt >= TASK_MAX) return NULL;

    task_tab[task_cnt] = (struct task_s*)mm_malloc(sizeof(struct task_s));
    if(task_tab[task_cnt] == NULL) return NULL;

    task_tab[task_cnt]->callback = cb;
    task_tab[task_cnt]->rate = interval;    
    task_tab[task_cnt]->last_run = 0;
    task_tab[task_cnt]->run = true;
    memcpy(task_tab[task_cnt]->name, name, TASK_NAME_MAX);
    task_tab[task_cnt]->name[TASK_NAME_MAX-1] = '\0';

    task_cnt++;

    return task_tab[task_cnt];
}

void task_set_rate(struct task_s* t, times_t time)
{
    t->rate = time;
}

void task_disable(struct task_s* t)
{
    t->run = false;
}

void task_print_list(void)
{
    PRINT("NAME\t\tCYCLE/us\tSTATUS\n");
    for(uint8_t i=0; i<task_cnt; i++) {
        PRINT("[%s]\t\t%lld\t\t%s\n", task_tab[i]->name, task_tab[i]->rate, task_tab[i]->run? "run":"idle");
    }
}

void scheduler_run(void)
{
    for(uint8_t i=0; i<task_cnt; i++) {
        if(task_tab[i]->run) {
//            PRINT("task:%d ",i);
            if(timer_check(&task_tab[i]->last_run, task_tab[i]->rate)) {
                task_tab[i]->callback();
                // PRINT("run \n");
            } else {
                // PRINT("wait \n");
            }        
        }
    }
}

void task_shell(int argc, char *argv[])
{
	if(argc == 2) {
		if(strcmp(argv[1],"list") == 0) {
			task_print_list();
			return;
		}
	}

	cli_device_write("missing command: try 'list'");
}
