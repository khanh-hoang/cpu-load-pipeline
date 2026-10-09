/*********************************************************************************************************************/
/*! \file sched_deadline.h
    \brief Wraps SCHED_DEADLINE setup for pipeline stage threads.
**********************************************************************************************************************/

#ifndef SCHED_DEADLINE_H
#define SCHED_DEADLINE_H

#include <stdint.h>
#include <sys/types.h>

/* Switches the calling thread to SCHED_DEADLINE. */
int sched_deadline_set(uint64_t runtime_ns, uint64_t deadline_ns,
                        uint64_t period_ns);

#endif // SCHED_DEADLINE_H
