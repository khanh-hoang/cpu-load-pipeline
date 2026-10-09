/*********************************************************************************************************************/
/*! \file job_queue.c
    \brief Blocking inter-stage queue with clean shutdown support.
**********************************************************************************************************************/

#include "job_queue.h"

#include <stdlib.h>
#include <string.h>

/* Initializes the ring buffer and synchronization primitives. */
int job_queue_init(job_queue_t *queue, size_t capacity) {
    memset(queue, 0, sizeof(*queue));
    queue->buffer = calloc(capacity, sizeof(job_record_t));
    if (!queue->buffer) {
        return -1;
    }
    queue->capacity = capacity;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        free(queue->buffer);
        return -1;
    }
    if (pthread_cond_init(&queue->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&queue->mutex);
        free(queue->buffer);
        return -1;
    }
    if (pthread_cond_init(&queue->not_full, NULL) != 0) {
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        free(queue->buffer);
        return -1;
    }
    return 0;
}

/* Frees queue resources. */
void job_queue_destroy(job_queue_t *queue) {
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
    pthread_mutex_destroy(&queue->mutex);
    free(queue->buffer);
    queue->buffer = NULL;
}

/* Pushes one record, blocking while the queue is full. */
int job_queue_push(job_queue_t *queue, const job_record_t *record) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == queue->capacity && !queue->shutdown) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }

    if (queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    queue->buffer[queue->tail] = *record;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

/* Pops one record, blocking while the queue is empty. */
int job_queue_pop(job_queue_t *queue, job_record_t *out_record) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }

    if (queue->count == 0 && queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }

    *out_record = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

/* Marks the queue shut down and wakes blocked push/pop callers. */
void job_queue_shutdown(job_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->shutdown = 1;
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
}
