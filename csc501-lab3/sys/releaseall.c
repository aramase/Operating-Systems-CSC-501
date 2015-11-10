#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

//for releasing all locks
int releaseall(int numlocks, int ldes1, ...)
{
    int ldes;
    int error_flag = 0;
    unsigned long *start = (unsigned long *)(&ldes1);
    for ( ; numlocks > 0 ; numlocks--)
    {
        ldes = *start++;
        
        if(release(currpid, ldes) == SYSERR)
            error_flag = 1;
    }
    
    resched();
    
    if(error_flag == 1)
        return(SYSERR);
    else
        return(OK);
}

int release(int pid, int ldes)
{
    STATWORD ps;
    register struct lockentry  *lptr;
    int chosen;
    int index = ldes % NLOCKS;
    
    disable(ps);
    
    //sanity check
    if (isbadlock(index) || (lptr= &locks[index])->lockstate==LFREE || lptr -> locknum != ldes)
    {
        restore(ps);
        return(SYSERR);
    }
    
    if(lockholdtab[pid][index] > 0)
        lockholdtab[pid][index]--;
    else
    {
        restore(ps);
        return(SYSERR);
    }
    
    if((lptr -> nreaders > 0) && (lptr -> nwriters == 0))
        lptr -> nreaders --;
    else if((lptr -> nreaders == 0) && (lptr -> nwriters == 1))
        lptr -> nwriters --;
    else
        //error-cannot get the lock
        
    if((lptr->nreaders == 0) && (lptr->nwriters == 0))
    {
        chosen = obtainlock(index);
        
        while(chosen != -1)
        {
            if(q[chosen].qtype == READ)
            {
                lptr -> nreaders ++;
                lockholdtab[chosen][index]++;
                dequeue(chosen);
                ready(chosen, RESCHNO);
                chosen = obtainlock(index);
                
                if((chosen != -1) && (q[chosen].qtype == WRITE))
                {
                    int item = q[lptr -> lockqtail].qprev;
                    while(item!=lptr -> lockqhead)
                    {
                        if((q[item].qkey >= q[chosen].qkey) && (q[item].qtype == READ))
                        {
                            chosen = item;
                            break;
                        }
                        item = q[item].qprev;
                    }
                    if(item == lptr -> lockqhead)
                        chosen = -1;
                }
            }
            else if(q[chosen].qtype == WRITE)
            {
                lptr -> nwriters ++;
                lockholdtab[chosen][index] ++;
                dequeue(chosen);
                ready(chosen, RESCHNO);
                break;
            }
        }
    }
    restore(ps);
    return OK;
}

