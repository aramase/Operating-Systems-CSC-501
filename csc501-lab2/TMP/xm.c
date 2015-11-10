/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */

SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
    /* sanity check ! */
    STATWORD ps;
    disable(ps);
    
    if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages >128) || bs_tab[source].bs_used_heap == 1)
    {
        restore(ps);
        return SYSERR;
    }
    
    //if current process already mapped
    if(proctab[currpid].map[source].bs_pid == currpid)
    {
        restore(ps);
        return(OK);
    }
    
    //otherwise need to add new mapping
    bs_add_mapping(source, currpid, virtpage, npages);
    
    restore(ps);
    return(OK);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
    STATWORD ps;
    disable(ps);
    
    /* sanity check ! */
    if ( (virtpage < 4096) )
    {
        restore(ps);
        return SYSERR;
    }
    
    //otherwise delete mapping
    bs_del_mapping(currpid, virtpage);
    
    pte_remove(virtpage, currpid);
    
    /*reload CR3 to clean up the mapping*/
    write_cr3(proctab[currpid].pdbr);
    
    restore(ps);
    return(OK);
    
}
