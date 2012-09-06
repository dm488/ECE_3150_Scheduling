#include "3140_concur.h"

realtime_t start;
realtime_t work;
realtime_t deadline;

void time_init(){
	start.sec = 0;
	start.msec = 0;
	work.sec = 0;
	work.msec = 0;
	deadline.sec = 0;
	deadline.msec = 0;
}

void delay (void)
{
	int i;
	for (i=0; i < 8000; i++) {
		__no_operation();
		__no_operation();
		__no_operation();
	}
}


void p1 (void)
{
	int i;
	for (i=0; i < 10; i++) {
		delay ();
		__disable_interrupt();
		P1OUT ^= 0x01;
		__enable_interrupt();
	}
}

void p2 (void)
{
	int i;
	for (i=0; i < 10; i++) {
		delay ();
		__disable_interrupt();
		P1OUT ^= 0x02;
		__enable_interrupt();
	}
}

void rt_p1 (void)
{
	int i;
	for (i=0; i < 10; i++) {
		delay ();
		__disable_interrupt();
		P1OUT = 0x01;
		__enable_interrupt();
	}
}

void rt_p2 (void)
{
	int i;
	for (i=0; i < 10; i++) {
		delay ();
		__disable_interrupt();
		P1OUT = 0x02;
		__enable_interrupt();
	}
}

int main (void){
	WDTCTL = WDTPW + WDTHOLD;
	P1DIR = 0x03;
	P1OUT = 0x00;	
	
	realtime_t_init();
	time_init();
	
	if (process_create (p1,10) < 0) {
	 	return -1;
	}
	if (process_create (p2,10) < 0) {
	 	return -1;
	}
	
	//rt processes	
	start.sec = 0;
	work.sec=1;
	deadline.msec=100;
	if (process_rt_create (rt_p1,10, start, work, deadline) < 0) {
	 	return -1;
	}
	start.sec = 0;
	work.sec=1;
	deadline.msec=90;
	if (process_rt_create (rt_p2,10, start, work, deadline) < 0) {
	 	return -1;
	}
	
	process_start();
	P1OUT= 0x02;
	while(1);
	return 0;	
}
	