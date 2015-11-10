/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <schedule.h>

extern int container;

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */

LOCAL int calc_goodness(int ppid)
{
        int goodness=-1;

        if(proctab[ppid].pcounter<=0)//if it has used up the complete quantum
            goodness = 0;
        else 
            goodness = proctab[ppid].pcounter+proctab[ppid].pprio;//carry over to the next epoch
        
        return goodness;
}

int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	
	if(proctab[pid].pflag == 0)
		insert(pid,rdyhead,calc_goodness(pid));
	else
	{
		if(proctab[pid].pcounter>0)
		insert(pid,real_rdyhead,container-1);
		else
		insert(pid,real_rdyhead,0);
	}
	
	
	if (resch)
		resched();
	return(OK);
}
