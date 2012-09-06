/* ECE 3140 Lab 5 */

#include "3140_concur.h"
#include <stdlib.h>

/* Timer A period controls the granularity of process interleavings */
#define TIMERA_PERIOD 50000
#define TIMERB_PERIOD 1000



/* Global process pointers */
process_t* current_process = NULL;	/* Currently-running process */
process_t* process_queue = NULL;	/* Points to head of process queue */

process_rt* rt_current_process = NULL;	/* Currently-running process */
process_rt* rt_process_queue = NULL;	/* Points to head of process queue */

/*Global timekeeper structure */
realtime_t current_time;


unsigned int missed_deadline = 0;

void realtime_t_init(){
	current_time.sec = 0;
	current_time.msec = 0;
}

/*------------------------------------------------------------------------
 * process_state 
 *   Bookkeeping structure, holds relevant information about process.
 *   (Declared and typedef'd to process_t in 3140_concur.h)
 *   Fields:
 *     sp   - current stack pointer for process
 *     next - In this implementation, process_state struct is also a
 *            linked-list node (for use in a process queue)
 * 	   blocked - Is it blocked by a lock?
 *     start - What was the start time?
 *     work - How long will it take to run?
 *     deadline - When is the deadline?
 *----------------------------------------------------------------------*/



/*------------------------------------------------------------------------
 * Process queue management convenience functions for CONCURRENCY
 *----------------------------------------------------------------------*/

/* Add process p to the tail of process queue */
void add_to_tail(process_t** head_ref, process_t* p) {
	/* Get pointer to the current head node */
	process_t* current = *head_ref;
	
	/* If queue is currently empty, replace it with the new process */
	if (current == NULL) { *head_ref = p; }
	
	/* Otherwise, find the end of the list and append the new node */
	else {
		while (current->next != NULL) { current = current->next; }
		/* At this point, current points to the last node in the list */
		current->next = p;
	}
}

/* Remove and return (pop) process from head of process queue */
process_t* take_from_head(process_t** head_ref) {
	/* We want to return the current head process */
	process_t* result = *head_ref;
	
	/* Remove the first process, unless the queue is empty */
	if (result != NULL) {
		*head_ref = result->next;	/* New head is the next process in queue */
		result->next = NULL;		/* Removed process no longer points to queue */
	}
	
	return result;
}


/*------------------------------------------------------------------------
 * Process queue management convenience functions for REALTIME
 *----------------------------------------------------------------------*/

/* Add process p to the tail of rt process queue */
void rt_add_to_tail(process_rt** head_ref, process_rt* p) {
	/* Get pointer to the current head node */
	process_rt* current_rt = *head_ref;
	
	/* If queue is currently empty, replace it with the new process */
	if (current_rt == NULL) { *head_ref = p; }
	
	/* Otherwise, find the end of the list and append the new node */
	else {
		while (current_rt->next != NULL) { current_rt = current_rt->next; }
		/* At this point, current points to the last node in the list */
		current_rt->next = p;
	}
}

/* Remove and return (pop) process from head of process queue */
process_rt* rt_take_from_head(process_rt** head_ref) {
	/* We want to return the current head process */
	process_rt* rt_result = *head_ref;
	
	/* Remove the first process, unless the queue is empty */
	if (rt_result != NULL) {
		*head_ref = rt_result->next;	/* New head is the next process in queue */
		rt_result->next = NULL;		/* Removed process no longer points to queue */
	}
	
	return rt_result;
}



/*------------------------------------------------------------------------
 *  process_create
 *    Allocate stack space for process, initialize bookkeeping structures
 *    Returns 0 if process is created successfully, -1 otherwise.
 * 
 *    f: pointer to function where the process should begin execution
 *    n: initial process stack size (in words)
 *----------------------------------------------------------------------*/
 
int process_create(void (*f)(void), int n) {
	/* Allocate bookkeeping structure for process */
	process_t* new_proc = (process_t*) malloc(sizeof(process_t));
	if (new_proc == NULL) { return -1; }	/* malloc failed */
	
	/* Allocate and initialize stack space for process */
	new_proc->sp = process_init(f, n);
	if (new_proc->sp == 0) { return -1; }	/* process_init failed */
	
	/* Add new process to process queue */
	new_proc->next = NULL;
	add_to_tail(&process_queue, new_proc);
	return 0;	/* Successfully created process and bookkeeping */
}
 
 /*---------------------------------------------------------------------------
  * process_rt_create - Creates a new process that arrives after 'start'
  * milliseconds have elapsed. Requires 'work' milliseconds to complete,
  * and has a relative 'deadline' milliseconds.
  * ------------------------------------------------------------------------*/
  
 int process_rt_create(void (*f)(void), int n, realtime_t start, realtime_t work, realtime_t deadline){
	unsigned int count = 0;
	process_rt *after = rt_process_queue->next;
	process_rt *before = rt_process_queue;
	/* Allocate bookkeeping structure for process */
	process_rt* new_proc_rt = (process_rt*) malloc(sizeof(process_rt));
	new_proc_rt->next = NULL;
	if (new_proc_rt == NULL) { return -1; }	/* malloc failed */
	
	/* Allocate and initialize stack space for process */
	new_proc_rt->sp = process_init(f, n);
	if (new_proc_rt->sp == 0) { return -1; }	/* process_init failed */
	
	/* Determine times for deadlines */
	new_proc_rt->start_s=start.sec;
	new_proc_rt->start_ms=start.msec;
	new_proc_rt->deadline_ms = new_proc_rt->start_ms + deadline.msec;
	if (new_proc_rt->deadline_ms >= 1000) {
		new_proc_rt->deadline_ms = new_proc_rt->deadline_ms - 1000;
		new_proc_rt->deadline_s++;
	}
	new_proc_rt->deadline_s = new_proc_rt->start_s + deadline.sec;
	
	/* Add new process to process queue */
	/* Determine where a process fits, based on EDF scheduling */
	if ((rt_process_queue == NULL)&&(new_proc_rt != NULL)){
		rt_process_queue = new_proc_rt;
		rt_process_queue->next=NULL;
	}
	else{
	while (before != NULL) {
		// if the new process has an earlier deadline than any task in the queue, move new_proc to higher priority and update queue
		if (1000*before->deadline_s + before->deadline_ms > 1000*new_proc_rt->deadline_s + new_proc_rt->deadline_ms) {
			if (count==0) {                 // replace first process in queue
				rt_add_to_tail(&new_proc_rt, before);
				rt_process_queue = new_proc_rt;
			}
			else{
			rt_add_to_tail(&new_proc_rt, after);
			before->next=NULL;
			rt_add_to_tail(&before, new_proc_rt);
			}
		}
		// move *before and *after one space right down the queue
		after = after->next;
		before = before->next;
		before->next = NULL;
		count++;
	}
 }
	return 0;	/* Successfully created process and bookkeeping */
}
 
 /*------------------------------------------------------------------------
  *  process_start
  *    Launch concurrent execution of processes (must be created first).
  *----------------------------------------------------------------------*/
  
void process_start(void) {
	/* Set up Timer A (triggers context switch) */
	TACCR0  = TIMERA_PERIOD;
	TACTL   = TACLR|MC_1|TASSEL_2|ID_2;
	TACCTL0 = CCIE;
	
	/* Set up Timer B (1000 cycles = 1 millisecond) */
  	TBCCR0 = TIMERB_PERIOD;
 	TBCTL = TBCLR|MC_1|TBSSEL_2|ID_3;
  	TBCCTL0 = CCIE;
	
	process_begin();	/* In assembly, actually launches processes */
}
 
  
 /*------------------------------------------------------------------------
  *  process_select
  *    Returns the stack pointer for the next process to execute, or 0
  *    if there are no more processes to run.
  * 
  *    cursp: stack pointer of currently running process, or 0 if there
  *           is no process currently running
  *----------------------------------------------------------------------*/

unsigned int process_select(unsigned int cursp) {
	/* cursp==0 -> No process currently running */
	process_rt *after = rt_process_queue->next;
	process_rt *now = rt_process_queue;
	process_rt *before = NULL;
	if (cursp == 0) {
		if (rt_process_queue!=NULL){
			TACTL   = TACLR|MC_0|TASSEL_2|ID_2;
			if ((rt_current_process!=NULL)&&((rt_current_process->deadline_s*1000 + rt_current_process->deadline_ms)<(current_time.sec*1000 + current_time.msec))) {
				missed_deadline++;
			}
			while (((now->start_s*1000 + now->start_ms)>(current_time.sec*1000 + current_time.msec))&&(now!=NULL)){
				before = now;
				now = after;
				after = after->next;
			}
			now->next = NULL;
			rt_current_process = now;
			if (before != NULL){before->next = after;}
			else {rt_process_queue = after;} 
			return rt_current_process->sp;
		}
		else {
			 TACTL = TACLR|MC_1|TASSEL_2|ID_2;
					
		
		
		
			current_process = NULL;
			if (process_queue == NULL) { return 0; }	/* No processes left */
			else {
				/* Return next process from queue */
				current_process = take_from_head(&process_queue);
				return current_process->sp;
			}
		}
	}
	/* cursp != 0 -> Some running process was interrupted */
	else {
		/* Save running process SP and add back to process queue to run later */
		current_process->sp = cursp;
		add_to_tail(&process_queue, current_process);
		/* Return next process from queue */
		current_process = take_from_head(&process_queue);
		return current_process->sp;
	}
}

/* Update current_time on every millisecond */
#pragma vector=TIMERB0_VECTOR
__interrupt void update_time(void)
{
  __disable_interrupt();
  current_time.msec++;
  if (current_time.msec >= 1000) {
  	current_time.msec = 0;
  	current_time.sec++;
  }
  __enable_interrupt();
}