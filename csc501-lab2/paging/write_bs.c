#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>

int write_bs(int src_frame, int dst_page) {
    
    STATWORD ps;
    disable(ps);
    
    unsigned long *source_addr = (unsigned long *)(src_frame*NBPG);
    unsigned long *destination_addr = (unsigned long *)(dst_page*NBPG);
    
    int a;
    for(a = 0; a < NBPG/sizeof(unsigned long); a++)
    {
        *destination_addr = *source_addr;
        destination_addr++;
        source_addr++;
    }
    
    restore(ps);
}

