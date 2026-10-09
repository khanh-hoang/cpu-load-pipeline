/*********************************************************************************************************************/
/*! \file csv_writer.c
    \brief Writes final CSV rows with derived timing and deadline fields.
**********************************************************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "csv_writer.h"
#include "timing_utils.h"

int csv_writer_write(const pipeline_config_t *cfg, const job_record_t *results,
                      long num_jobs) {
    FILE *file = fopen(cfg->csv_path, "w");
    if (!file) {
        fprintf(stderr, "failed to open %s: %s\n", cfg->csv_path, strerror(errno));
        return -1;
    }

    fprintf(file,
        "job_id,warmup,"
        "t_input_s,"
        "queue1_wait_us,stage1_exec_us,"
        "queue2_wait_us,stage2_exec_us,"
        "queue3_wait_us,stage3_exec_us,"
        "queue4_wait_us,"
        "e2e_latency_us,e2e_deadline_miss,"
        "input_cpu,stage1_cpu,stage2_cpu,stage3_cpu,output_cpu\n");

    for (long i = 0; i < num_jobs; i++) {
        const job_record_t *record = &results[i];

        // Skip jobs that never made it through the pipeline
        if (record->t_output.tv_sec == 0 && record->t_output.tv_nsec == 0) {
            continue;
        }

        double queue1_wait_us = timing_diff_us(&record->t_input, &record->t_stage_start[0]);
        double stage1_exec_us = timing_diff_us(&record->t_stage_start[0], &record->t_stage_end[0]);
        double queue2_wait_us = timing_diff_us(&record->t_stage_end[0], &record->t_stage_start[1]);
        double stage2_exec_us = timing_diff_us(&record->t_stage_start[1], &record->t_stage_end[1]);
        double queue3_wait_us = timing_diff_us(&record->t_stage_end[1], &record->t_stage_start[2]);
        double stage3_exec_us = timing_diff_us(&record->t_stage_start[2], &record->t_stage_end[2]);
        double queue4_wait_us = timing_diff_us(&record->t_stage_end[2], &record->t_output);
        double e2e_latency_us = timing_diff_us(&record->t_input, &record->t_output);

        int e2e_deadline_miss = (e2e_latency_us / 1000.0) > cfg->e2e_deadline_ms;
        int warmup = (record->job_id < cfg->warmup_jobs);

        fprintf(file,
            "%ld,%d,"
            "%.9f,"
            "%.3f,%.3f,"
            "%.3f,%.3f,"
            "%.3f,%.3f,"
            "%.3f,"
            "%.3f,%d,"
            "%d,%d,%d,%d,%d\n",
            record->job_id, warmup,
            timing_ts_to_seconds(&record->t_input),
            queue1_wait_us, stage1_exec_us,
            queue2_wait_us, stage2_exec_us,
            queue3_wait_us, stage3_exec_us,
            queue4_wait_us,
            e2e_latency_us, e2e_deadline_miss,
            record->input_cpu, record->stage_cpu[0], record->stage_cpu[1], record->stage_cpu[2],
            record->output_cpu);
    }

    fclose(file);
    return 0;
}
