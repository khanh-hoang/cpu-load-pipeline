/*********************************************************************************************************************/
/*! \file timing_utils.c
    \brief Provides timespec conversion, difference, and increment helpers.
**********************************************************************************************************************/

#include "timing_utils.h"

double timing_ts_to_seconds(const struct timespec *ts) {
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}

double timing_diff_us(const struct timespec *start, const struct timespec *end) {
    // Difference first, then scale, to avoid precision loss from uptime-sized doubles.
    long long sec = (long long)end->tv_sec - (long long)start->tv_sec;
    long long nsec = (long long)end->tv_nsec - (long long)start->tv_nsec;
    return (double)(sec * 1000000LL) + (double)nsec / 1000.0;
}

void timing_add_ns(struct timespec *ts, long ns) {
    ts->tv_nsec += ns;
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec += 1;
    }
}
