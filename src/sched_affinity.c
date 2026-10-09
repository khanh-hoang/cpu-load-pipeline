/*********************************************************************************************************************/
/*! \file sched_affinity.c
    \brief Wraps CPU affinity setup for pipeline thread pinning.
**********************************************************************************************************************/

#define _GNU_SOURCE

#include "sched_affinity.h"

#include <sched.h>

int sched_affinity_set(unsigned int cpu_mask) {
    if (cpu_mask == 0) {
        return 0;
    }
    cpu_set_t set;
    CPU_ZERO(&set);
    for (int cpu = 0; cpu_mask; cpu++, cpu_mask >>= 1) {
        if (cpu_mask & 1u) {
            CPU_SET(cpu, &set);
        }
    }
    // pid 0 means "the calling thread itself".
    return sched_setaffinity(0, sizeof(set), &set);
}
