/* signal.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include<lab0.h>

/*------------------------------------------------------------------------
 * signal  --  signal a semaphore, releasing one waiting process
 *------------------------------------------------------------------------
 */
SYSCALL signal(int sem)
{

//changes for incrementing count on syscall
int start_time;
if(call_active == 1) 
{
call_used[currpid] = 1;
call_frequency[currpid][3]++;//id for signal()=3
start_time = ctr1000;
}

	STATWORD ps;    
	register struct	sentry	*sptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		return(SYSERR);
	}
	if ((sptr->semcnt++) < 0)
		ready(getfirst(sptr->sqhead), RESCHYES);
	restore(ps);

if(call_active == 1) 
{
call_duration[currpid][3] += ctr1000 - start_time;
}
return(OK);
}
