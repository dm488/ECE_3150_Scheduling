/*************************************************************************
 *
 *  Copyright (c) 2011 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  $Id$
 *
 **************************************************************************
 */
#ifndef __3140_CONCUR_H__
#define __3140_CONCUR_H__

#include "msp430x22x4.h"

#ifndef NULL
#define NULL 0
#endif

//typedef struct realtime_t
//	/* keep track of time

typedef struct time {
  unsigned int sec;
  unsigned int msec;
}realtime_t;

//typedef struct process_state process_t;
//   /* opaque definition of process type; you must provide this
//      implementation.
//   */
typedef struct process_state {
	unsigned int sp;				/* Stack pointer for process */
	struct process_state* next;		/* Pointer to next process in queue */
}process_t;

typedef struct rt_process_state {
	unsigned int sp;				/* Stack pointer for process */
	unsigned int start_s;           /* start time (sec) */
	unsigned int start_ms;          /* start time (msec) */
	unsigned int work_s;            /* run time (sec) */
	unsigned int work_ms;           /* run time (msec) */
	unsigned int deadline_s;        /* relative deadline (sec) */
	unsigned int deadline_ms;       /* relative deadline (msec) */
	struct rt_process_state* next;		/* Pointer to next process in queue */
}process_rt;

/*------------------------------------------------------------------------

   THE FOLLOWING FUNCTIONS MUST BE PROVIDED.

------------------------------------------------------------------------*/

/* ====== Concurrency ====== */

unsigned int process_select (unsigned int cursp);
/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/

void realtime_t_init (void);
void time_init(void);

extern process_t *current_process; 
/* the currently running process */

extern process_t *process_queue;
/* the head node of ready process queue */

void process_start (void);
/* Starts up the concurrent execution */

int process_create (void (*f)(void), int n);
/* Create a new process */

int process_rt_create (void (*f)(void), int n, realtime_t start, realtime_t work, realtime_t deadline);
/* Create a new process */

/*------------------------------------------------------------------------
  
You may use the following functions that we have provided

------------------------------------------------------------------------*/


/* This function can ONLY BE CALLED if interrupts are disabled.
   This function switches execution to the next ready process, and is
   also the entry point for the timer interrupt.
   
   Implemented in 3140.asm
*/
void process_blocked (void);

/*
  This function is called by user code indirectly when the process
  terminates. This is handled by stack manipulation.

  Implemented in 3140.asm
  Used in 3140_concur.c
*/
extern void process_terminated (void);

/* This function can ONLY BE CALLED if interrupts are disabled. It
   does not modify interrupt flags.
*/
unsigned int process_init (void (*f)(void), int n);

/*
  This function starts the concurrency by using the timer interrupt
  context switch routine to call the first ready process.

  The function also gracefully exits once the process_select()
  function returns 0.

  This must be called with interrupts disabled.
*/
void process_begin (void);

//MODIFIED CODE to include process.c functions

void add_to_tail(process_t** head_ref, process_t* p);

process_t* take_from_head(process_t** head_ref);

void rt_add_to_tail(process_rt** head_ref, process_rt* p);

process_rt* rt_take_from_head(process_rt** head_ref);

#endif /* __3140_CONCUR_H__ */
