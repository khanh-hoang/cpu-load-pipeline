/*********************************************************************************************************************/
/*! \file csv_writer.h
    \brief Writes final CSV rows with derived timing and deadline fields.
**********************************************************************************************************************/

#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "job_record.h"
#include "pipeline_config.h"

int csv_writer_write(const pipeline_config_t *cfg, const job_record_t *results,
                      long num_jobs);

#endif // CSV_WRITER_H
