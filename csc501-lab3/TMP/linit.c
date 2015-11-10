#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>

struct lockentry locks[NLOCKS];
int nextlock;
int lockholdtab[NPROC][NLOCKS];

void linit()
{
    struct lockentry *lptr;
    int a, b;
    nextlock = NLOCKS-1;
    
    for(a = 0; a < NLOCKS; a++)
    {
        (lptr = &locks[a])->lockstate = LFREE;
        lptr -> locknum = a;
        lptr -> lockqtail = 1 + (lptr -> lockqhead = newqueue());
        lptr -> nreaders = 0;
        lptr -> nwriters = 0;
    }
    
    for(a = 0; a < NPROC; a ++)
        for(b = 0; b < NLOCKS; b ++)
            lockholdtab[a][b] = 0;
}
