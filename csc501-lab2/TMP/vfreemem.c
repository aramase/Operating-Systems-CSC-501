/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
WORD *block;
unsigned size;
{
    STATWORD ps;
    disable(ps);
    
    if(size <=0 || block == NULL)
    {
        restore(ps);
        return(SYSERR);
    }
    
    
    size = (unsigned)roundmb(size);
    
    virtmemblock *current = (virtmemblock *)getmem(sizeof(virtmemblock));
    current = proctab[currpid].freememlist;
    virtmemblock *previous = (virtmemblock *)getmem(sizeof(virtmemblock));
    
    previous->vaddr = (unsigned long)block;
    previous->vlen = size;
    previous->num = current->num+1;
    current->num = 0;
    previous->next = current;
    
    proctab[currpid].freememlist = previous;
    
    restore(ps);
    return(OK);
}
