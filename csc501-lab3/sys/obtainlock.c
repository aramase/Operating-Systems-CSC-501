#include <q.h>
#include <lock.h>
#include <sleep.h>

int obtainlock(int lock)
{
    struct lockentry *lptr = &locks[lock];
    int chosen, temp;
    
    temp = q[lptr -> lockqtail].qprev;
    chosen = q[lptr -> lockqtail].qprev;
    
    if(chosen == lptr -> lockqhead)
    {
        return -1;
    }
    
    while(q[temp].qprev != lptr -> lockqhead)
    {
        temp = q[temp].qprev;
        if(q[temp].qkey < q[chosen].qkey)
            break;
        else if(q[temp].qkey > q[chosen].qkey)
        {
            //kprintf("Wrong! priority inverse!\n");
        }
        else
        {
            int wait1 = clktime - q[chosen].qwait;
            int wait2 = clktime - q[temp].qwait;
            if( ((wait1 - wait2) <= 1) && ((wait1 - wait2) >= -1))
            {
                if((q[chosen].qtype == READ) && (q[temp].qtype == WRITE))
                    chosen = temp;
                else
                    chosen = chosen;
            }
            else if(wait1 > wait2)
                chosen = chosen;
            else if(wait1 < wait2)
                chosen = temp;
        }
    }
    return chosen;
}
