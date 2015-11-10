#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>
#include <sleep.h>

int lock(int ldes1, int type, int priority)
{
    STATWORD ps;
    struct  lockentry  *lptr;
    struct  pentry  *pptr;
    int item, writer_flag = 0;
    int index = ldes1 % NLOCKS;
    
    disable(ps);
    
    //sanity check
    if (isbadlock(index) || (lptr= &locks[index])->lockstate==LFREE || lptr -> locknum != ldes1)
    {
        restore(ps);
        return(SYSERR);
    }
    
    if((lptr -> nreaders == 0) && (lptr -> nwriters == 0))
        writer_flag = 0;
    else if((lptr -> nreaders > 0) && (lptr -> nwriters == 0))
    {
        writer_flag = 0;
        if(type == READ)
        {
            for(item = q[lptr -> lockqtail].qprev; (item != lptr -> lockqhead) && (priority < q[item].qkey); item = q[item].qprev)
            {
                if(q[item].qtype == WRITE)
                {
                    writer_flag = 1;
                    break;
                }
            }
        }
        else if(type == WRITE)
            writer_flag = 1;
    }
    else if((lptr -> nreaders == 0) && (lptr -> nwriters == 1))
        writer_flag = 1;
    else
        //cannot get a lock
        
    if(writer_flag)
    {
        (pptr = &proctab[currpid]) -> pstate = PRLWAIT;
        pptr -> plock = ldes1;
        pptr -> plockwaitret = OK;
        
        insert(currpid, lptr -> lockqhead, priority);
        
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
            lptr -> nreaders++;
        
        else if(type == WRITE)
            lptr -> nwriters++;
        
        else
            //Do nothing
        restore(ps);
        return(OK);
    }
}

