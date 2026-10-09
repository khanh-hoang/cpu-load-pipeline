/*********************************************************************************************************************/
/*! \file main.c
    \brief Orchestrates configuration, threads, queues, CSV output, and cleanup.
**********************************************************************************************************************/

#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "csv_writer.h"
#include "job_queue.h"
#include "pipeline_config.h"
#include "pipeline_threads.h"

int main(int argc, char **argv) {
    // Avoid first-touch page faults during measured stage execution.
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        perror("mlockall (continuing; expect page-fault outliers)");
    }

    // Defaults first, then CLI flags override them.
    pipeline_config_t config;
    pipeline_config_set_defaults(&config);
    if (pipeline_config_parse_args(argc, argv, &config) != 0) {
        return EXIT_FAILURE;
    }

    // Fixed job count for one preallocated results array.
    long num_jobs = (long)(config.duration_sec * config.rate_hz);
    if (num_jobs <= 0) {
        fprintf(stderr, "computed num_jobs <= 0, check --duration/--rate\n");
        return EXIT_FAILURE;
    }

    // Zeroed records let csv_writer skip incomplete jobs.
    job_record_t *results = calloc((size_t)num_jobs, sizeof(job_record_t));
    if (!results) {
        fprintf(stderr, "failed to allocate results array\n");
        return EXIT_FAILURE;
    }

    // One queue per hop: input->stage1->stage2->stage3->output.
    job_queue_t queues[4];
    for (int i = 0; i < 4; i++) {
        if (job_queue_init(&queues[i], config.queue_capacity) != 0) {
            fprintf(stderr, "failed to init queue %d\n", i);
            // Destroy only queues that initialized successfully.
            for (int j = 0; j < i; j++) {
                job_queue_destroy(&queues[j]);
            }
            free(results);
            return EXIT_FAILURE;
        }
    }

    // Wires the input generator's only queue to Q1.
    input_thread_ctx_t input_ctx = {
        .out_queue = &queues[0],
        .rate_hz = config.rate_hz,
        .num_jobs = num_jobs,
        .fifo_enabled = config.input_fifo,
        .sched_fifo_failed = 0,
        .pin_enabled = config.pin_enabled,
        .sched_affinity_failed = 0,
    };

    // Each stage reads from Q[i] and writes to Q[i+1].
    stage_thread_ctx_t stage_ctx[NUM_STAGES];
    for (int i = 0; i < NUM_STAGES; i++) {
        stage_ctx[i] = (stage_thread_ctx_t){
            .stage_index = i,
            .in_queue = &queues[i],
            .out_queue = &queues[i + 1],
            .runtime_ns = config.runtime_ns,
            .deadline_ns = config.deadline_ns,
            .period_ns = config.period_ns,
            .iterations = config.iterations,
            .sched_deadline_failed = 0,
            .pin_enabled = config.pin_enabled,
            .sched_affinity_failed = 0,
        };
    }

    // Output collector stores completed jobs in release order.
    output_thread_ctx_t output_ctx = {
        .in_queue = &queues[3],
        .results = results,
        .completed = 0,
        .fifo_enabled = config.output_fifo,
        .sched_fifo_failed = 0,
        .pin_enabled = config.pin_enabled,
        .sched_affinity_failed = 0,
    };

    // Start consumers before producers.
    pthread_t output_tid, stage_tid[NUM_STAGES], input_tid;
    int create_failed = 0;

    if (pthread_create(&output_tid, NULL, output_thread_main, &output_ctx) != 0) {
        fprintf(stderr, "failed to create output thread\n");
        create_failed = 1;
    }
    for (int i = NUM_STAGES - 1; i >= 0 && !create_failed; i--) {
        if (pthread_create(&stage_tid[i], NULL, stage_thread_main, &stage_ctx[i]) != 0) {
            fprintf(stderr, "failed to create stage %d thread\n", i);
            create_failed = 1;
        }
    }
    if (!create_failed &&
        pthread_create(&input_tid, NULL, input_thread_main, &input_ctx) != 0) {
        fprintf(stderr, "failed to create input thread\n");
        create_failed = 1;
    }

    if (create_failed) {
        // Do not free queues while already-started threads may still use them.
        fprintf(stderr, "aborting: not all pipeline threads could be started\n");
        return EXIT_FAILURE;
    }

    // Join in shutdown cascade order.
    pthread_join(input_tid, NULL);
    for (int i = 0; i < NUM_STAGES; i++) {
        pthread_join(stage_tid[i], NULL);
    }
    pthread_join(output_tid, NULL);

    // Report scheduling setup failures before writing results.
    for (int i = 0; i < NUM_STAGES; i++) {
        if (stage_ctx[i].sched_deadline_failed) {
            fprintf(stderr,
                    "WARNING: stage %d never switched to SCHED_DEADLINE; "
                    "results for this run are not valid for analysis\n", i);
        }
        if (stage_ctx[i].sched_affinity_failed) {
            fprintf(stderr, "WARNING: stage %d CPU pinning failed\n", i);
        }
    }
    if (input_ctx.sched_fifo_failed) {
        fprintf(stderr, "WARNING: input thread never switched to SCHED_FIFO\n");
    }
    if (output_ctx.sched_fifo_failed) {
        fprintf(stderr, "WARNING: output thread never switched to SCHED_FIFO\n");
    }
    if (input_ctx.sched_affinity_failed) {
        fprintf(stderr, "WARNING: input thread CPU pinning failed\n");
    }
    if (output_ctx.sched_affinity_failed) {
        fprintf(stderr, "WARNING: output thread CPU pinning failed\n");
    }
    fprintf(stderr, "collected %ld / %ld jobs\n", output_ctx.completed, num_jobs);

    // Keep file I/O off the measurement path.
    int csv_result = csv_writer_write(&config, results, num_jobs);

    for (int i = 0; i < 4; i++) {
        job_queue_destroy(&queues[i]);
    }
    free(results);

    return (csv_result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
