#ifndef MYQUEUE_6YUN7OAZ
#define MYQUEUE_6YUN7OAZ
#ifdef __cplusplus
extern "C" { /*}*/
#endif

struct queue {
   unsigned char head;
   unsigned char tail;
   unsigned short size;
   unsigned char *buf;
};
void queue_init(struct queue *q, unsigned char *buf, unsigned char size);

/* 0:succ
 * 1: failed*/
extern unsigned char queue_out(struct queue *q, unsigned char *data);

/* 0:succ
 * 1: failed*/
extern unsigned char queue_in(struct queue *q, unsigned char data);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: MYQUEUE_6YUN7OAZ */
