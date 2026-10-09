/*********************************************************************************************************************/
/*! \file sched_fifo.h
	\brief Wraps SCHED_FIFO setup for input and output threads.
**********************************************************************************************************************/

#ifndef SCHED_FIFO_H
#define SCHED_FIFO_H

/* Switches the calling thread to SCHED_FIFO. */
int sched_fifo_set(int priority);

#endif // SCHED_FIFO_H
