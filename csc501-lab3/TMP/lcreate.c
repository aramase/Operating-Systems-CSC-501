#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int getnewlock();

int lcreate(void)
{
    STATWORD ps;
    int lock;
    
    disable(ps);
    if ((lock=getnewlock())==SYSERR)
    {
        restore(ps);
        return(SYSERR);
    }
    
    restore(ps);
    return(lock);//return the lockdesc
}

LOCAL int getnewlock()
{
    int temp,a,b;
    for (a=0 ; a<NLOCKS ; a++)
    {
        temp=nextlock--;
        if (nextlock < 0)
            nextlock = NLOCKS-1;
        if (locks[temp].lockstate==LFREE)//lock available
        {
            locks[temp].lockstate = LUSED;
            locks[temp].locknum = locks[temp].locknum + NLOCKS;
            locks[temp].nreaders = 0;
            locks[temp].nwriters = 0;
            
            for(b = 0; b < NPROC; b++)
                lockholdtab[b][temp] = 0;
            
            return(locks[temp].locknum);
        }
    }
    return(SYSERR);
}

