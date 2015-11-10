#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <stdio.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
    
    /* requests a new mapping of npages with ID map_id */
    
    STATWORD ps;
    disable(ps);
    
    if(npages == 0 || npages > 128 || bs_id < 0 || bs_id > MAX_ID || bs_tab[bs_id].bs_used_heap == 1)
    {
        restore(ps);
        return(SYSERR);
    }
    
    else if(bs_tab[bs_id].bs_status == BSM_MAPPED)
    {
        restore(ps);
        return(bs_tab[bs_id].bs_npages);//return number of pages
    }
    
    //bs is empty
    bs_tab[bs_id].map_pid = NULL;
    bs_tab[bs_id].bs_status = BSM_MAPPED;
    bs_tab[bs_id].bs_npages = npages;
    bs_tab[bs_id].bs_used_heap = 0;
    
    restore(ps);
    return npages;
    
}


