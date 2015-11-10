/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
unsigned nbytes;
{
    
    STATWORD ps;
    
    disable(ps);
    
    if (nbytes == 0 || proctab[currpid].freememlist->num == 0)
    {
        restore(ps);
        return ((WORD *)SYSERR);
    }
    
    nbytes = (unsigned int) roundmb(nbytes);
    
    virtmemblock *currentblk = (virtmemblock *)getmem(sizeof(virtmemblock));
    virtmemblock *prev = (virtmemblock *)getmem(sizeof(virtmemblock));
    
    currentblk = proctab[currpid].freememlist;
    prev = currentblk;
    
    int i = 0;
    while(i < proctab[currpid].freememlist->num)
    {
        if(currentblk->vlen == nbytes)
        {
            if(i == 0)
            {
                unsigned long ret_addr = currentblk->vaddr;
                if(proctab[currpid].freememlist->num == 1)
                {
                    proctab[currpid].freememlist->vaddr = 0;
                    proctab[currpid].freememlist->num = 0;
                    proctab[currpid].freememlist->vlen = 0;
                }
                else
                {
                    proctab[currpid].freememlist->vaddr = currentblk->next->vaddr;
                    proctab[currpid].freememlist->num = (currentblk->num)-1;
                    proctab[currpid].freememlist->vlen = currentblk->next->vlen;
                }
                
                restore(ps);
                return((WORD *)ret_addr);
            }
            else
            {
                prev->next = currentblk->next;
                proctab[currpid].freememlist->num--;
                proctab[currpid].freememlist->vlen -= nbytes;
                
                restore(ps);
                return((WORD *)currentblk->vaddr);
            }
            
        }
        else if(currentblk->vlen > nbytes)
        {
            unsigned long ret_addr = currentblk->vaddr;
            
            currentblk->vaddr += nbytes;
            currentblk->vlen -= nbytes;
            
            
            restore(ps);
            return((WORD *)ret_addr);
        }
        prev = currentblk;
        currentblk = currentblk->next;
        i++;
    }
    
    restore(ps);
    return NULL;
}


