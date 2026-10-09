/*********************************************************************************************************************/
/*! \file pipeline_config.h
    \brief Defines run settings and CLI parsing for one pipeline experiment.
**********************************************************************************************************************/

#ifndef PIPELINE_CONFIG_H
#define PIPELINE_CONFIG_H

#include <stddef.h>
#include <stdint.h>

/* Tunable settings for one pipeline run. */
typedef struct {
    double duration_sec;
    double rate_hz;
    long iterations;
    uint64_t runtime_ns;
    uint64_t deadline_ns;
    uint64_t period_ns;
    long warmup_jobs;
    double e2e_deadline_ms; 
    size_t queue_capacity;   
    int input_fifo;
    int output_fifo;
    int pin_enabled;
    char csv_path[512];
} pipeline_config_t;

void pipeline_config_set_defaults(pipeline_config_t *cfg);

/* Parses CLI options into cfg. Returns 0 on success, -1 on error. */
int pipeline_config_parse_args(int argc, char **argv, pipeline_config_t *cfg);

#endif // PIPELINE_CONFIG_H
