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
#include "3140_concur.h"
#include <stdlib.h>	

/*
  State layout:

  .-----------------.
  |       retpc     |
  |-----------------|
  |       pc        |
  |-----------------|
  |       sr        |
  |-----------------|
  |       R4        |
  |-----------------|
  |      ....       |
  |                 |
  |-----------------|
  |      R15        | <---- sp
  '-----------------'

  State requires 15 slots on the stack.

  The rest of the registers are automatically saved when
  process_switch() is called, since they are caller-saved registers!
*/


/*------------------------------------------------------------------------
 *
 *  process_init --
 *
 *   Initialize process state
 *
 *------------------------------------------------------------------------
 */
unsigned int 
process_init (void (*f)(void), int n)
{
  int *sp;	/* Pointer to process stack (allocated in heap) */ 
  int i;

  /* in reality, there are 14 more slots needed */
  n += 15;

  /* Allocate space for the process's stack */
  sp = (int *) malloc(n*sizeof(int));
  if (sp == NULL) { return 0; }	/* Allocation failed */
  
  /* Initialize the stack to all zeros */ 
  /* Note: Could just use calloc instead */ 
  for (i=0; i < n; i++) {
  	sp[i] = 0;
  }
  
  /* Highest stack address holds the address we should jump to
   * after the process terminates. process_terminated is
   * a pointer-to-a-function, so we cast it to int first. */
  sp[n-1] = (unsigned int) process_terminated;
  
  /* Put the function entry point and an initial status register
   * at the next highest stack address. This emulates an
   * interrupt, and allows us to start executing the function
   * by doing a reti */
  sp[n-2] = (unsigned int) f;
  sp[n-3] = GIE;	/* Initialize SR to only have GIE bit set */
  
  return (unsigned int)&(sp[n-15]);
}
