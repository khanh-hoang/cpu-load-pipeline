/*********************************************************************************************************************/
/*! \file sched_affinity.h
	\brief Wraps CPU affinity setup for pipeline thread pinning.
**********************************************************************************************************************/

#ifndef SCHED_AFFINITY_H
#define SCHED_AFFINITY_H

/* Pins the calling thread to CPUs selected by cpu_mask. */
int sched_affinity_set(unsigned int cpu_mask);

#endif // SCHED_AFFINITY_H
