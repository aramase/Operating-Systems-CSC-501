#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <sleep.h>

int lock(int ldes1, int type, int priority)
{
    STATWORD ps;
    int item, writer_flag = 0;
    int index = ldes1 % NLOCKS;
	
	//struct  lockentry  *lptr=&locks[index];
	int testElement;
    
    disable(ps);
    
    //sanity check
    if (isbadlock(index) || (locks[index].lockstate==LFREE) || locks[index].locknum != ldes1)
    {
        restore(ps);
        return(SYSERR);
    }
    
    if((locks[index].nreaders == 0) && (locks[index].nwriters == 0))
        writer_flag = 0;
	
    else if((locks[index].nreaders > 0) && (locks[index].nwriters == 0))
    {
        //writer_flag = 0;
        if(type == READ)
        {
			testElement=q[locks[index].lockqtail].qprev;
			
			while(priority<q[testElement].qkey)
            {
                if(q[testElement].qtype == WRITE)
                {
                    writer_flag = 1;
		    testElement=q[testElement].qprev;
                }
            }
        }
		
        else if(type == WRITE)
            writer_flag = 1;
    }
	
	else if(locks[index].nreaders==0 && locks[index].nwriters>0)
			writer_flag=1;
		
    if(writer_flag==1)
    {
		struct  pentry  *pptr=&proctab[currpid];

        pptr-> pstate = PRLWAIT;
        pptr -> plock = ldes1;
        pptr -> plockwaitret = OK;
        
        insert(currpid, locks[index].lockqhead, priority);
        
        q[currpid].qtype = type;
        q[currpid].qwait = clktime;
        resched();
        
        restore(ps);
        return pptr->plockwaitret;
    }
    else
    {
        lockholdtab[currpid][index]++;
        
        if(type == READ)
            locks[index].nreaders++;
        else if(type == WRITE)
            locks[index].nwriters++;
        restore(ps);
        return(OK);
    }
}

