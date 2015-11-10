/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include<lab0.h>

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep(int n)
{

//changes for incrementing count on syscall
int start_time;
if(call_active == 1) 
{
call_used[currpid] = 1;
call_frequency[currpid][4]++;//id for sleep()=4
start_time = ctr1000;
}

	STATWORD ps;    
	if (n<0 || clkruns==0)
		return(SYSERR);
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);

if(call_active == 1) 
{
call_duration[currpid][4] += ctr1000 - start_time;
}
return(OK);
}
