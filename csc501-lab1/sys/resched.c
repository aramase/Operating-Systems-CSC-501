/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <schedule.h>
#include <stdio.h>
extern unsigned long ctr1000;
unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
extern int current_queue_flag;
extern int container;

/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:       Upon entry, currpid gives current process id.
 *      Proctab[currpid].pstate gives correct NEXT state for
 *      current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
 
 int calc_goodness(int ppid)
{
        int goodness=-1;

        if(proctab[ppid].pcounter<=0)//if it has used up the complete quantum
            goodness = 0;
        else 
            goodness = proctab[ppid].pcounter+proctab[ppid].pprio;//carry over to the next epoch
        
        return goodness;
}

int resched()
{
    register struct pentry  *optr;  /* pointer to old process entry */
    register struct pentry  *nptr;  /* pointer to new process entry */
	int i;
	int next_epoch=0;
	int next_real = 0;
	int temp;
	int count=0;
	int chosen=0;
	int goodness;
	int reset = 0;
	
	 if(getschedclass() == MULTIQSCHED) 
	{
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].pflag==0)
			{
				if(proctab[i].pcounter==0 );
				else if(proctab[i].pstate==PRSUSP || proctab[i].pstate==PRFREE);
				else
					next_epoch++;
				 
			}
			else
			{
				if(proctab[i].pcounter==0 );
				else if(proctab[i].pstate==PRSUSP || proctab[i].pstate==PRFREE);
				else
					next_real++;
			}
		}
		
		if((next_epoch == 0 && current_queue_flag == 0) || (next_real==0 && current_queue_flag == 1))
		{
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pflag == 0 && proctab[i].pquantum != proctab[i].pcounter)
				reset = 1;
				else
				continue;
			}
			container = 100;
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pflag == 0 && reset ==1)
				{
					proctab[i].pcounter = proctab[i].pprio + floor(proctab[i].pcounter/2);
					proctab[i].pquantum = proctab[i].pprio + floor(proctab[i].pcounter/2);
				}
				else
				{
					proctab[i].pquantum =100;
					proctab[i].pcounter =100;
				}
			}
			while(nonempty(real_rdyhead))
			{
				i=getlast(real_rdytail);
			}
			
			while(nonempty(rdyhead))
			{
				i=getlast(rdytail);
			}
			
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pstate==PRREADY && proctab[i].pflag==1)
				{
					insert(i,real_rdyhead,100);
				}
			}
			
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pstate==PRREADY && proctab[i].pflag==0)
				{
					goodness=calc_goodness(i);
					insert(i,rdyhead,goodness);
				}
			}
		
			chosen=srand(100)%100;
		
			if(chosen>=30)
			{
				if(nonempty(real_rdyhead))
				{
					current_queue_flag=1;//set real queue
				}
				else
				{
					current_queue_flag=0;//for normal queue
				}
			}

			else if(chosen<30)
			{
				//to calc the number of ready process in normal queue
				count = 0;	
				for(i=0;i<NPROC;i++)
				{
					if(proctab[i].pstate==PRREADY && proctab[i].pflag==0)
					{
						count+=1;
					}
				}

				if(count>1)//making sure null is not the only process
				{
					current_queue_flag=0;
				}
				else if(nonempty(real_rdyhead))
				{
					current_queue_flag=1;
				}
				else
					current_queue_flag=0;//only null is present
			}
		}
		
		if((next_epoch != 0 && current_queue_flag == 0) || (next_real!=0 && current_queue_flag == 1))
		{
			if(preempt>0)
			proctab[currpid].pcounter = preempt;
			else
			proctab[currpid].pcounter = 0;
		}
		
		if(current_queue_flag == 1)
		{
			if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
           (lastkey(rdytail)<0)) 
		   {
                return(OK);
			}

			/* force context switch */

			if (optr->pstate == PRCURR) 
			{
                optr->pstate = PRREADY;
                if(proctab[currpid].pcounter>0)
				insert(currpid,real_rdyhead,--container);
				else
				insert(currpid,real_rdyhead,0);
			}

        /* remove highest priority process at end of ready list */

        nptr = &proctab[ (currpid = getlast(real_rdytail)) ];
		nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
        preempt = proctab[currpid].pcounter;              /* reset preemption counter     */
#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;
		}
		else
		{
			temp = calc_goodness(currpid);
		 
			if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
           (lastkey(rdytail)<temp)) 
		    {
                return(OK);
			}

			/* force context switch */

			if (optr->pstate == PRCURR) 
			{
                optr->pstate = PRREADY;
                insert(currpid,rdyhead,calc_goodness(currpid));
			}

			/* remove highest priority process at end of ready list */

			nptr = &proctab[ (currpid = getlast(rdytail)) ];
			nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
			preempt = proctab[currpid].pcounter;              /* reset preemption counter     */
#endif

			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

			/* The OLD process returns here when resumed. */
			return OK;

		}
		
	}
	
	else
	{
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].pcounter == 0);
			else if(proctab[i].pstate == PRSUSP || proctab[i].pstate == PRFREE);
			else
				next_epoch++;
			
		}
		
		if(next_epoch == 0)
		{
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pstate==PRREADY || proctab[i].pstate==PRCURR || proctab[i].pstate==PRSUSP)
				{
					proctab[i].pquantum=proctab[i].pprio+floor(proctab[i].pcounter/2);
					proctab[i].pcounter=proctab[i].pprio+floor(proctab[i].pcounter/2);
				}
			}
			
			while(nonempty(rdyhead))
			{
				i=getlast(rdytail);
			}
			
			for(i=0;i<NPROC;i++)
			{
				if(proctab[i].pstate==PRREADY)
				{
					insert(i,rdyhead,calc_goodness(i));
				}
			}
			
		}
		
		if(next_epoch !=0)
		{
			if(preempt>0)
				proctab[currpid].pcounter = preempt;
			else
				proctab[currpid].pcounter = 0;
		}
		
		 temp = calc_goodness(currpid);
		 
		 if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
           (lastkey(rdytail)<temp)) {
                return(OK);
        }

        /* force context switch */

        if (optr->pstate == PRCURR) {
                optr->pstate = PRREADY;
                insert(currpid,rdyhead,calc_goodness(currpid));
        }

        /* remove highest priority process at end of ready list */

        nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;          /* mark it currently running    */
#ifdef  RTCLOCK
        preempt = proctab[currpid].pcounter;              /* reset preemption counter     */
#endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;

		
		
	}
	
}
