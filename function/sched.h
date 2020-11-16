#ifndef SCHED_EK67SSTG
#define SCHED_EK67SSTG
#ifdef __cplusplus
extern "C" {
/*}*/
#endif



#define	ENABLE_TASK_STATUS                                  0

#define MAX_TASKS                                           4
#define USE_TASK_RET                                        1



#define TASK_INVALID_ID                                     255
#define TASK_RUN                                            0
#define TASK_SUSPEND                                        1
#define TASK_PREEMPT_DISABLED                               0
#define TASK_PREEMPT_ENABLED                                !TASK_PREEMPT_DISABLED


#define TASK_RET_NORMAL                                     0
#define TASK_RET_SUSPEND                                    1
#define TASK_RET_INVALD                                     255

/*#if MAX_TASKS > 255
typedef	unsigned short task_type_t;
#else
typedef	unsigned char task_type_t;
#endif*/

#if USE_TASK_RET
unsigned char sched_task_add(unsigned char (*fun)(void), const unsigned int delay,
		const unsigned int period, const int preempt);
#else
unsigned char sched_task_add(void (*fun)(void), const unsigned int delay,
		const unsigned int period, const int preempt);
#endif


extern void sched_task_ticks(void);

extern void sched_task_run(void);


#define sched_task_init()

#define task_start(pre, next)                               for(;;) { \
    pre;\
    sched_task_run();\
    next;\
}

/*extern inline unsigned char is_has_run_task(void);*/

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: SCHED_EK67SSTG */
