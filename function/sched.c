#include "sched.h"

typedef struct {
#if USE_TASK_RET
    unsigned char
#else
    void
#endif
     (*fun)(void);
    unsigned int delay;
    unsigned int period;
    unsigned int preempt: 1; /*preempt enable */
#if ENABLE_TASK_STATUS
    unsigned int status: 1;	/* run count*/
    unsigned int run: 6;	/* run count*/
#else
    unsigned int run: 7;	/* run count*/
#endif
    /*unsigned char times;[>force indicate run times<]*/
} pcb_t;

static pcb_t sch_tasks[MAX_TASKS];

#define run_times_update()/*	{ if (sch_tasks[i].times != TASK_RUNTIMES_INFINITY)\*/
	/*--sch_tasks[i].times; }*/
#define run_times_chk()/*{ if (sch_tasks[i].times == 0) sch_tasks[i].period = 0; } */

void sched_task_ticks(void)
{
    unsigned char i;
    for (i = 0; i < MAX_TASKS; i++) {
#if ENABLE_TASK_STATUS
        if (sch_tasks[i].fun && sch_tasks[i].status == TASK_RUN) {
#else
        if (sch_tasks[i].fun) {
#endif
            if (sch_tasks[i].delay-- == 0) {
                if (sch_tasks[i].preempt != TASK_PREEMPT_DISABLED) { /*preempt enabled*/
                    sch_tasks[i].fun();
                    run_times_update();
                }
                sch_tasks[i].run += 1;
                run_times_chk();
                if (sch_tasks[i].period)
                    sch_tasks[i].delay = sch_tasks[i].period;
            }
        }
    }
}
void sched_task_run(void)
{
    unsigned char i;
#if USE_TASK_RET
    unsigned char flag = TASK_RET_NORMAL;
#endif
    for (i = 0; i < MAX_TASKS; i++
#if USE_TASK_RET
            ,flag = TASK_RET_NORMAL
#endif
            ) {
#if ENABLE_TASK_STATUS
        if (sch_tasks[i].run > 0 && sch_tasks[i].status == TASK_RUN) {
#else
        if (sch_tasks[i].run > 0) {
#endif
            if (sch_tasks[i].fun && sch_tasks[i].preempt == TASK_PREEMPT_DISABLED)
#if USE_TASK_RET
                flag =
#endif
                    sch_tasks[i].fun();  // run the task
#if ENABLE_TASK_STATUS
#if USE_TASK_RET
            if (flag == TASK_RET_SUSPEND)
                sch_tasks[i].status = TASK_SUSPEND;
#endif
#endif
            sch_tasks[i].run--;
            run_times_update();
#if USE_TASK_RET
            if (!sch_tasks[i].period || flag == TASK_RET_INVALD) {
                sch_tasks[i].fun = 0;/*del*/
            }
#endif
        }
    }
}
/*inline unsigned char is_has_run_task(void)
  {
  unsigned char val;
  unsigned int i;
  for (val = 0, i = 0; i < MAX_TASKS; i++) {
  if (sch_tasks[i].run) {
  val = sch_tasks[i].run;
  break;
  }
  }
  return val;
  }*/

#if USE_TASK_RET
unsigned char sched_task_add(unsigned char (*fun)(void), const unsigned int delay,
            const unsigned int period, const int preempt)
#else
unsigned char sched_task_add(void (*fun)(void), const unsigned int delay,
            const unsigned int period, const int preempt)
#endif
{
    unsigned char i = 0;

    /*find a gap in the tasks*/
    while (sch_tasks[i].fun && i < MAX_TASKS)
        i++;

    if (i == MAX_TASKS)
        return 1;

    sch_tasks[i].fun  = fun;

    sch_tasks[i].delay  = delay;
    sch_tasks[i].period = period;
    sch_tasks[i].preempt  = preempt;
    sch_tasks[i].run  = 0;

#if ENABLE_TASK_STATUS
    sch_tasks[i].status  = TASK_RUN;
#else
#endif

    return i;
}
