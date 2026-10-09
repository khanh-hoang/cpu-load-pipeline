/*********************************************************************************************************************/
/*! \file pipeline_config.c
    \brief Defines run settings and CLI parsing for one pipeline experiment.
**********************************************************************************************************************/

#include "pipeline_config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

/* Sets default run parameters. */
void pipeline_config_set_defaults(pipeline_config_t *cfg) {
    cfg->duration_sec = 60.0;
    cfg->rate_hz = 100.0;
    cfg->iterations = 200000;
    cfg->runtime_ns = 800ULL * 1000;
    cfg->deadline_ns = 3000ULL * 1000;
    cfg->period_ns = 10000ULL * 1000; 
    cfg->warmup_jobs = 200;
    cfg->e2e_deadline_ms = 10.0;
    cfg->queue_capacity = 64;
    cfg->input_fifo = 0;
    cfg->output_fifo = 0;
    cfg->pin_enabled = 0;
    snprintf(cfg->csv_path, sizeof(cfg->csv_path), "results.csv");
}

/* Prints supported command-line options. */
static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  --duration <sec>       total run duration (default 60)\n"
        "  --rate <jobs/sec>      fixed job release rate (default 100)\n"
        "  --iterations <n>       workload loop count per stage (default 200000)\n"
        "  --runtime-us <n>       SCHED_DEADLINE runtime per stage, in us (default 800)\n"
        "  --deadline-us <n>      SCHED_DEADLINE deadline per stage, in us (default 3000)\n"
        "  --period-us <n>        SCHED_DEADLINE period per stage, in us (default 10000)\n"
        "  --warmup <n>           jobs flagged as warmup (default 200)\n"
        "  --e2e-deadline-ms <n>  end-to-end budget checked per job (default 10)\n"
        "  --queue-capacity <n>   per-hop queue capacity (default 64)\n"
        "  --input-fifo           switch the input generator to SCHED_FIFO (default: off, SCHED_OTHER)\n"
        "  --output-fifo          switch the output collector to SCHED_FIFO (default: off, SCHED_OTHER)\n"
        "  --pin                  pin all pipeline threads to cores 1-3 (default: off, unpinned)\n"
        "  --output <path>        CSV output path (default results.csv)\n"
        "  -h, --help             show this help\n",
        prog);  
}

int pipeline_config_parse_args(int argc, char **argv, pipeline_config_t *cfg) {
    // Keep long-only option values clear of ASCII short-option values.
    enum {
        OPT_DURATION = 1000, OPT_RATE, OPT_ITERATIONS, OPT_RUNTIME_US,
        OPT_DEADLINE_US, OPT_PERIOD_US, OPT_WARMUP, OPT_E2E_DEADLINE_MS,
        OPT_QUEUE_CAP, OPT_OUTPUT, OPT_INPUT_FIFO, OPT_OUTPUT_FIFO,
        OPT_PIN
    };
    static struct option long_opts[] = {
        {"duration",        required_argument, 0, OPT_DURATION},
        {"rate",            required_argument, 0, OPT_RATE},
        {"iterations",      required_argument, 0, OPT_ITERATIONS},
        {"runtime-us",      required_argument, 0, OPT_RUNTIME_US},
        {"deadline-us",     required_argument, 0, OPT_DEADLINE_US},
        {"period-us",       required_argument, 0, OPT_PERIOD_US},
        {"warmup",          required_argument, 0, OPT_WARMUP},
        {"e2e-deadline-ms", required_argument, 0, OPT_E2E_DEADLINE_MS},
        {"queue-capacity",  required_argument, 0, OPT_QUEUE_CAP},
        {"output",          required_argument, 0, OPT_OUTPUT},
        {"input-fifo",      no_argument,       0, OPT_INPUT_FIFO},
        {"output-fifo",     no_argument,       0, OPT_OUTPUT_FIFO},
        {"pin",             no_argument,       0, OPT_PIN},
        {"help",            no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    // Each recognized flag overrides the default set earlier.
    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_opts, NULL)) != -1) {
        switch (opt) {
            // Convert user-facing microseconds to internal nanoseconds.
            case OPT_DURATION:        cfg->duration_sec = atof(optarg); break;
            case OPT_RATE:            cfg->rate_hz = atof(optarg); break;
            case OPT_ITERATIONS:      cfg->iterations = atol(optarg); break;
            case OPT_RUNTIME_US:      cfg->runtime_ns = strtoull(optarg, NULL, 10) * 1000ULL; break;
            case OPT_DEADLINE_US:     cfg->deadline_ns = strtoull(optarg, NULL, 10) * 1000ULL; break;
            case OPT_PERIOD_US:       cfg->period_ns = strtoull(optarg, NULL, 10) * 1000ULL; break;
            case OPT_WARMUP:          cfg->warmup_jobs = atol(optarg); break;
            case OPT_E2E_DEADLINE_MS: cfg->e2e_deadline_ms = atof(optarg); break;
            case OPT_QUEUE_CAP:       cfg->queue_capacity = (size_t)strtoul(optarg, NULL, 10); break;
            case OPT_OUTPUT:          snprintf(cfg->csv_path, sizeof(cfg->csv_path), "%s", optarg); break;
            case OPT_INPUT_FIFO:      cfg->input_fifo = 1; break;
            case OPT_OUTPUT_FIFO:     cfg->output_fifo = 1; break;
            case OPT_PIN:             cfg->pin_enabled = 1; break;
            case 'h':
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    // Basic positive-value validation.
    if (cfg->duration_sec <= 0 || cfg->rate_hz <= 0 || cfg->iterations <= 0) {
        fprintf(stderr, "duration, rate, and iterations must all be > 0\n");
        return -1;
    }

    // SCHED_DEADLINE requires runtime <= deadline <= period.
    if (cfg->runtime_ns == 0 ||
        cfg->runtime_ns > cfg->deadline_ns ||
        cfg->deadline_ns > cfg->period_ns) {
        fprintf(stderr,
                "require 0 < runtime <= deadline <= period "
                "(got runtime=%lluus deadline=%lluus period=%lluus)\n",
                (unsigned long long)(cfg->runtime_ns / 1000),
                (unsigned long long)(cfg->deadline_ns / 1000),
                (unsigned long long)(cfg->period_ns / 1000));
        return -1;
    }
    return 0;
}
