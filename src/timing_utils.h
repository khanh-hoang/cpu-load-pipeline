/*********************************************************************************************************************/
/*! \file timing_utils.h
	\brief Provides timespec conversion, difference, and increment helpers.
**********************************************************************************************************************/

#ifndef TIMING_UTILS_H
#define TIMING_UTILS_H

#include <time.h>

double timing_ts_to_seconds(const struct timespec *ts);

double timing_diff_us(const struct timespec *start, const struct timespec *end);

void timing_add_ns(struct timespec *ts, long ns);

#endif // TIMING_UTILS_H
