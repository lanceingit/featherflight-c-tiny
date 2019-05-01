/**                                               _____           ,-.
 * _______       _____       _____                ___   _,.      /  /
 * ___    |__   ____(_)_____ __  /______________  __   ; \____,-==-._  )
 * __  /| |_ | / /_  /_  __ `/  __/  __ \_  ___/  _    //_    `----' {+>
 * _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /      _    `  `'--/  /-'`(
 * /_/  |_|____/ /_/  \__,_/ \__/ \____//_/       _          /  /
 *                                                           `='
 * 
 * scheduler.c
 *
 * v1.1
 *
 * Simple scheduling system, task form
 */
#include "board.h"
#include "scheduler.h"
#include "debug.h"
#include "mm.h"
#include <string.h>

Task* task_tab[TASK_MAX] = {NULL};
static uint8_t task_cnt=0;

void task_shell(int argc, char* argv[]);

void task_print_list(void)
{
    PRINT("NAME\t\tCYCLE/us\tSTATUS\n");
    for(uint8_t i=0; i<task_cnt; i++) {
        if(strlen(task_tab[i]->name) > 6) {
            PRINT("[%s]\t%lld\t\t%s\n", task_tab[i]->name, task_tab[i]->rate, task_tab[i]->run? "run":"idle");
        }
        else {
            PRINT("[%s]\t\t%lld\t\t%s\n", task_tab[i]->name, task_tab[i]->rate, task_tab[i]->run? "run":"idle");
        }
    }
}

void task_set_rate(Task* t, times_t time)
{
    t->rate = time;
}

void task_disable(Task* t)
{
    t->run = false;
}

Task* task_create(char* name, times_t interval, task_callback_func cb)
{
    if(task_cnt >= TASK_MAX) return NULL;

    task_tab[task_cnt] = (Task*)mm_malloc(sizeof(Task));
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

void scheduler_run(void)
{
    for(uint8_t i=0; i<task_cnt; i++) {
        if(task_tab[i]->run) {
//            PRINT("task:%d ",i);
            if(timer_check(&task_tab[i]->last_run, task_tab[i]->rate)) {
                task_tab[i]->callback();
                // PRINT("run \n");
            }
            else {
                // PRINT("wait \n");
            }
        }
    }
}

void task_init(void)
{
    cli_regist("task", task_shell);
}

void task_shell(int argc, char* argv[])
{
    if(argc == 2) {
        if(strcmp(argv[1], "list") == 0) {
            task_print_list();
            return;
        }
    }

    cli_device_write("missing command: try 'list'");
}
