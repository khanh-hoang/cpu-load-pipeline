/*********************************************************************************************************************/
/*! \file workload.c
    \brief Implements the fixed-count synthetic stage workload.
**********************************************************************************************************************/

#include "workload.h"

/* Prevents the workload loop from being optimized away. */
static volatile double g_workload_sink;

void workload_run(long iterations) {
    double acc = 1.0;
    const double a = 1.0000001;
    const double b = 0.9999999;

    for (long i = 0; i < iterations; i++) {
        acc = acc * a + b;
        // Keep acc bounded during long runs.
        if (acc > 1e6) {
            acc = 1.0;
        }
    }

    g_workload_sink = acc;
}
