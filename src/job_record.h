/*********************************************************************************************************************/
/*! \file job_record.h
    \brief Defines the raw timestamp record carried through the pipeline.
**********************************************************************************************************************/

#ifndef JOB_RECORD_H
#define JOB_RECORD_H

#include <time.h>

#define NUM_STAGES 3

/* Raw timestamps and CPU ids for one pipeline job. */
typedef struct {
    long job_id;
    struct timespec t_input;
    struct timespec t_stage_start[NUM_STAGES];
    struct timespec t_stage_end[NUM_STAGES];
    struct timespec t_output;
    int input_cpu;
    int stage_cpu[NUM_STAGES];
    int output_cpu;
} job_record_t;

#endif // JOB_RECORD_H
