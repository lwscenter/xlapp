#include "myqueue.h"
void queue_init(struct queue *q, unsigned char *buf, unsigned char size)
{
    q->size = size;
    q->head = q->tail = 0;
    /*10*/
    q->buf = buf;
}
/* 0:succ
 * 1: failed*/
unsigned char queue_out(struct queue *q, unsigned char *data)
{
     if(q->head == q->tail)
        return 1;
    *data = q->buf[q->head];
    q->head = (q->head + 1) % q->size;
    return 0;
}
/* 0:succ
 * 1: failed*/
unsigned char queue_in(struct queue *q, unsigned char data)
{
    if(q->head == (q->tail + 1) % q->size)
        return 1;
    q->buf[q->tail] = data;
    q->tail = (q->tail + 1) % q->size;
    return 0;
}


