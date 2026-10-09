/*********************************************************************************************************************/
/*! \file pipeline_threads.h
    \brief Declares the input, stage, and output thread contexts and entry points.
**********************************************************************************************************************/

#ifndef PIPELINE_THREADS_H
#define PIPELINE_THREADS_H

#include <stdint.h>

#include "job_queue.h"
#include "job_record.h"

/* Input generator thread context. */
typedef struct {
    job_queue_t *out_queue;
    double rate_hz;
    long num_jobs;
    int fifo_enabled;
    int sched_fifo_failed;
    int pin_enabled;
    int sched_affinity_failed;
} input_thread_ctx_t;

/* SCHED_DEADLINE stage thread context. */
typedef struct {
    int stage_index;
    job_queue_t *in_queue;
    job_queue_t *out_queue;
    uint64_t runtime_ns;
    uint64_t deadline_ns;
    uint64_t period_ns;
    long iterations;
    int sched_deadline_failed;
    int pin_enabled;
    int sched_affinity_failed;
} stage_thread_ctx_t;

/* Output collector thread context. */
typedef struct {
    job_queue_t *in_queue;
    job_record_t *results;
    long completed;
    int fifo_enabled;
    int sched_fifo_failed;
    int pin_enabled;
    int sched_affinity_failed;
} output_thread_ctx_t;

void *input_thread_main(void *arg);
void *stage_thread_main(void *arg);
void *output_thread_main(void *arg);

#endif // PIPELINE_THREADS_H
