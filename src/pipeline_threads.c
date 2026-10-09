/*********************************************************************************************************************/
/*! \file pipeline_threads.c
    \brief Implements the input, stage, and output pipeline thread loops.
**********************************************************************************************************************/

#define _GNU_SOURCE

#include "pipeline_threads.h"

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "timing_utils.h"
#include "workload.h"
#include "sched_deadline.h"
#include "sched_fifo.h"
#include "sched_affinity.h"

#define PIPELINE_FIFO_PRIORITY 1
#define PIPELINE_PIN_MASK 14u

void *input_thread_main(void *arg) {
    input_thread_ctx_t *ctx = (input_thread_ctx_t *)arg;

    // Optional SCHED_FIFO priority for input timing protection.
    if (ctx->fifo_enabled && sched_fifo_set(PIPELINE_FIFO_PRIORITY) != 0) {
        fprintf(stderr,
                "input: sched_fifo_set failed: %s\n", strerror(errno));
        ctx->sched_fifo_failed = 1;
    }
    // Optional pinning to the pipeline CPU set.
    if (sched_affinity_set(ctx->pin_enabled ? PIPELINE_PIN_MASK : 0) != 0) {
        fprintf(stderr, "input: sched_affinity_set failed: %s\n", strerror(errno));
        ctx->sched_affinity_failed = 1;
    }

    // Convert release rate to an absolute-period step.
    long period_ns = (long)(1e9 / ctx->rate_hz);

    // First release anchor; later releases are computed by addition.
    struct timespec next_release;
    clock_gettime(CLOCK_MONOTONIC, &next_release);

    for (long job_id = 0; job_id < ctx->num_jobs; job_id++) {
        job_record_t record;
        // Zeroed timestamps let csv_writer skip incomplete jobs.
        memset(&record, 0, sizeof(record));
        record.job_id = job_id;
        clock_gettime(CLOCK_MONOTONIC, &record.t_input);
        record.input_cpu = sched_getcpu();

        if (job_queue_push(ctx->out_queue, &record) != 0) {
            break;  // downstream shut down early
        }

        // Absolute sleep avoids release drift.
        timing_add_ns(&next_release, period_ns);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_release, NULL);
    }

    // Shutdown cascades downstream from the input queue.
    job_queue_shutdown(ctx->out_queue);
    return NULL;
}

void *stage_thread_main(void *arg) {
    stage_thread_ctx_t *ctx = (stage_thread_ctx_t *)arg;

    // Pin before SCHED_DEADLINE admission.
    if (sched_affinity_set(ctx->pin_enabled ? PIPELINE_PIN_MASK : 0) != 0) {
        fprintf(stderr, "stage %d: sched_affinity_set failed: %s\n",
                ctx->stage_index, strerror(errno));
        ctx->sched_affinity_failed = 1;
    }

    // Apply the reservation once for the thread lifetime.
    if (sched_deadline_set(ctx->runtime_ns, ctx->deadline_ns, ctx->period_ns) != 0) {
        fprintf(stderr,
                "stage %d: sched_deadline_set failed: %s "
                "(run as root / with CAP_SYS_NICE)\n",
                ctx->stage_index, strerror(errno));
        // Keep running, but mark the run invalid for deadline analysis.
        ctx->sched_deadline_failed = 1;
    }

    job_record_t record;
    while (job_queue_pop(ctx->in_queue, &record) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &record.t_stage_start[ctx->stage_index]);
        record.stage_cpu[ctx->stage_index] = sched_getcpu();
        workload_run(ctx->iterations);
        clock_gettime(CLOCK_MONOTONIC, &record.t_stage_end[ctx->stage_index]);

        if (job_queue_push(ctx->out_queue, &record) != 0) {
            break;
        }
    }

    // Cascade shutdown to the next hop.
    job_queue_shutdown(ctx->out_queue);
    return NULL;
}

void *output_thread_main(void *arg) {
    output_thread_ctx_t *ctx = (output_thread_ctx_t *)arg;

    // Optional SCHED_FIFO priority for output timing protection.
    if (ctx->fifo_enabled && sched_fifo_set(PIPELINE_FIFO_PRIORITY) != 0) {
        fprintf(stderr,
                "output: sched_fifo_set failed: %s\n", strerror(errno));
        ctx->sched_fifo_failed = 1;
    }
    // Optional pinning to the pipeline CPU set.
    if (sched_affinity_set(ctx->pin_enabled ? PIPELINE_PIN_MASK : 0) != 0) {
        fprintf(stderr, "output: sched_affinity_set failed: %s\n", strerror(errno));
        ctx->sched_affinity_failed = 1;
    }

    job_record_t record;
    while (job_queue_pop(ctx->in_queue, &record) == 0) {
        clock_gettime(CLOCK_MONOTONIC, &record.t_output);
        record.output_cpu = sched_getcpu();
        // Index by job_id to keep release order.
        ctx->results[record.job_id] = record;
        ctx->completed++;
    }
    return NULL;
}
