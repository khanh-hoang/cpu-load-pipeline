/*********************************************************************************************************************/
/*! \file job_queue.h
    \brief Blocking inter-stage queue with clean shutdown support.
**********************************************************************************************************************/

#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>
#include <stddef.h>

#include "job_record.h"

/* Fixed-capacity blocking queue between pipeline stages. */
typedef struct {
    job_record_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    int shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} job_queue_t;

int  job_queue_init(job_queue_t *queue, size_t capacity);
void job_queue_destroy(job_queue_t *queue);

/* Blocks while full. Returns 0 on success, -1 if shut down. */
int job_queue_push(job_queue_t *queue, const job_record_t *record);

/* Blocks while empty and not shut down. Returns 0 on success, -1 if shut down. */
int job_queue_pop(job_queue_t *queue, job_record_t *out_record);

/* Marks the queue as shut down and wakes any waiters. */
void job_queue_shutdown(job_queue_t *queue);

#endif // JOB_QUEUE_H
