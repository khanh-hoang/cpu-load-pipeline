/*********************************************************************************************************************/
/*! \file sched_fifo.c
    \brief Wraps SCHED_FIFO setup for input and output threads.
**********************************************************************************************************************/

#include "sched_fifo.h"

#include <sched.h>

int sched_fifo_set(int priority) {
    struct sched_param param = { .sched_priority = priority };
    // pid 0 means "the calling thread itself".
    return sched_setscheduler(0, SCHED_FIFO, &param);
}
