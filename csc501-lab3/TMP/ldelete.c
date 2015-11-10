#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int ldelete(int lockdescriptor)
{
    STATWORD ps;
    int     pid, a;
    struct  lockentry  *lptr;
    int index = lockdescriptor % NLOCKS;
    
    disable(ps);
    
    //sanity check
    if (isbadlock(index) || ((lptr=&locks[index])->lockstate==LFREE) || (lptr -> locknum != lockdescriptor))
    {
        restore(ps);
        return(SYSERR);
    }
    
    //make it free
    lptr->lockstate = LFREE;
    
    if(nonempty(lptr->lockqhead))
    {
        while( (pid=getfirst(lptr->lockqhead)) != EMPTY)
        {
            proctab[pid].plockwaitret = DELETED;
            ready(pid,RESCHNO);
        }
        
        resched();
    }
    
    //reinitializing num of readers and writers to 0
    lptr -> nreaders = 0;
    lptr -> nwriters = 0;
    
    for(a = 0; a < NPROC; a++)
        lockholdtab[a][index] = 0;
    
    restore(ps);
    return(OK);
}

