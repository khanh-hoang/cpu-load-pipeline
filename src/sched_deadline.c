/*********************************************************************************************************************/
/*! \file sched_deadline.c
    \brief Wraps SCHED_DEADLINE setup for pipeline stage threads.
**********************************************************************************************************************/

#include "sched_deadline.h"

#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

/* Fallback policy id for older toolchains. */
#ifndef SCHED_DEADLINE
#define SCHED_DEADLINE 6
#endif

/* Kernel ABI layout for sched_setattr(2). */
struct sched_attr {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t  sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

/* Raw sched_setattr(2) syscall wrapper. */
static int sched_setattr(pid_t pid, const struct sched_attr *attr,
                          unsigned int flags) {
    return (int)syscall(SYS_sched_setattr, pid, attr, flags);
}

int sched_deadline_set(uint64_t runtime_ns, uint64_t deadline_ns,
                        uint64_t period_ns) {
    struct sched_attr attr;
    memset(&attr, 0, sizeof(attr));

    // Tell the kernel which sched_attr layout this call uses.
    attr.size = sizeof(attr);
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_flags = 0;
    attr.sched_runtime = runtime_ns;
    attr.sched_deadline = deadline_ns;
    attr.sched_period = period_ns;

    // pid 0 means "the calling thread itself".
    if (sched_setattr(0, &attr, 0) != 0) {
        return -1;
    }
    return 0;
}
