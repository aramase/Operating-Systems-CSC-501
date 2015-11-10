/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_map[NFRAMES];
frame_t frm_tab[NFRAMES];
frame_t *fifo_head, *fifo_tail;
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL init_frm()
{
    STATWORD ps;
    disable(ps);
    int i;
    for(i = 0; i < NFRAMES; i++)
    {
        frm_tab[i].status = FRM_FREE;
        frm_tab[i].refcnt = 0;
        frm_tab[i].bs = -1;
        frm_tab[i].bs_page = -1;
        frm_tab[i].bs_next = NULL;
        frm_tab[i].fifo = NULL;
        frm_tab[i].age = 0;
        frm_tab[i].frame_num = i+FRAME0;
        
        frm_map[i].fr_status = FRM_UNMAPPED;
        frm_map[i].fr_pid = -1;
        frm_map[i].fr_vpno = -1;
        frm_map[i].fr_refcnt = 0;
        frm_map[i].fr_type = FR_PAGE;
        frm_map[i].fr_dirty = -1;
        frm_map[i].shared = NULL;
        frm_map[i].bs_page_num = -1;
        frm_map[i].frame_num = i+FRAME0;
        
    }
    
    fifo_head = NULL;
    fifo_tail = NULL;
    restore(ps);
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int frame)
{
    STATWORD ps;
    disable(ps);
    if(frame < 4 || frame > NFRAMES || frm_tab[frame].status == FRM_PGD || frm_tab[frame].status == FRM_PGT || frm_tab[frame].status == FRM_FREE)
    {
        restore(ps);
        return(SYSERR);
    }
    
    //write frame to backing store page
    write_bs(frm_tab[frame].frame_num, frm_tab[frame].bs_page);
    
    //remove the entry from the NI page table
    page_table[frm_tab[frame].bs_page-total_bs_available] = -1;
    
    frm_tab[frame].status = FRM_FREE;
    frm_tab[frame].refcnt = 0;
    frm_tab[frame].bs = -1;
    frm_tab[frame].bs_page = -1;
    //frm_tab[frame].bs_next = NULL;
    frm_tab[frame].age = 0;
    
    //remove this frame from fifo list
    frame_t *next, *prev;
    next = fifo_head;
    if(next == &frm_tab[frame])
    {
        fifo_head = fifo_head->fifo;
    }
    else
    {
        while(next->fifo != NULL)
        {
            if(next == &frm_tab[frame])
            {
                prev->fifo = next->fifo;
            }
            prev = next;
            next = next->fifo;
        }
        
    }
    frm_tab[frame].fifo = NULL;
    
    //remove reference from page table. Now here we can have multiple vpnos. Hence need to loop
    int flag = 0;
    do
    {
        unsigned long int pd_offset = frm_map[frame].fr_vpno >>10;
        unsigned long int pt_offset = frm_map[frame].fr_vpno;
        pt_offset = pt_offset & 1023;
        
        pd_t *pt_frame = (pd_t *)(proctab[frm_map[frame].fr_pid].pdbr);
        pt_frame += pd_offset;
        
        unsigned int pg_table = pt_frame->pd_base;
        
        pt_t *ptr = (pt_t *)(pg_table*NBPG);
        ptr += pt_offset;
        
        ptr->pt_pres = 0;
        ptr->pt_write = 0;
        ptr->pt_user = 0;
        ptr->pt_pwt = 0;
        ptr->pt_pcd = 0;
        ptr->pt_acc = 0;
        ptr->pt_dirty = 0;
        ptr->pt_mbz = 0;
        ptr->pt_global = 0;
        ptr->pt_avail = 0;
        ptr->pt_base = 0;
        
        //decrement page table ref count
        frm_map[pg_table-FRAME0].fr_refcnt--;
        frm_tab[pg_table-FRAME0].refcnt--;
        
        frm_map[frame].fr_status = FRM_UNMAPPED;
        //frm_map[frame].fr_pid = -1;
        //frm_map[frame].fr_vpno = -1;
        frm_map[frame].fr_refcnt = 0;
        frm_map[frame].fr_type = FR_PAGE;
        frm_map[frame].fr_dirty = -1;
        //frm_map[frame].shared = NULL;
        frm_map[frame].bs_page_num = -1;
        
        if(frm_map[frame].shared == NULL)
        {
            frm_map[frame].fr_pid = -1;
            frm_map[frame].fr_vpno = -1;
            flag = 1;
        }
        else
        {
            frm_map[frame].fr_pid = frm_map[frame].shared->fr_pid;
            frm_map[frame].fr_vpno = frm_map[frame].shared->fr_vpno;
            frm_map[frame].shared = frm_map[frame].shared->shared;
        }
        
    } while(!flag);
    
    frm_map[frame].shared = NULL;
    restore(ps);
    return OK;
}

void pte_remove(int vpno, int pid)
{
    unsigned long int pd_offset = vpno >>10;
    unsigned long int pt_offset = vpno;
    pt_offset = pt_offset & 1023;
    
    //get the page table frame
    pd_t *pt_frame = (pd_t *)(proctab[pid].pdbr);
    pt_frame += pd_offset;
    
    unsigned int pg_table = pt_frame->pd_base;
    
    //checking page table ref cnt
    if(frm_tab[pg_table-FRAME0].refcnt <= 0 && pg_table > FRAME0+5)
    {
        int i = pg_table-FRAME0;
        frm_tab[i].status = FRM_FREE;
        frm_tab[i].refcnt = 0;
        frm_tab[i].bs = -1;
        frm_tab[i].bs_page = -1;
        frm_tab[i].bs_next = NULL;
        frm_tab[i].fifo = NULL;
        frm_tab[i].age = 0;
        
        frm_map[i].fr_status = FRM_UNMAPPED;
        frm_map[i].fr_pid = -1;
        frm_map[i].fr_vpno = -1;
        frm_map[i].fr_refcnt = 0;
        frm_map[i].fr_type = FR_PAGE;
        frm_map[i].fr_dirty = -1;
        frm_map[i].shared = NULL;
        frm_map[i].bs_page_num = -1;
    }
    
    //removing from page directory
    pt_frame->pd_pres = 0;
    pt_frame->pd_write = 0;
    pt_frame->pd_user = 0;
    pt_frame->pd_pwt = 0;
    pt_frame->pd_pcd = 0;
    pt_frame->pd_acc = 0;
    pt_frame->pd_mbz = 0;
    pt_frame->pd_fmb = 0;
    pt_frame->pd_global = 0;
    pt_frame->pd_avail = 0;
    pt_frame->pd_base = 0;
    
}


int get_frm()
{
    STATWORD ps;
    
    disable(ps);
    
    int i;
    
    for(i = 0; i < NFRAMES; i++)
    {
        if(frm_tab[i].status == FRM_FREE)
        {
            restore(ps);
            return (i+FRAME0);
        }
    }
    
    restore(ps);
    return(page_replace());
}

int page_replace()
{
    STATWORD ps;
    disable(ps);
    
    int tlb_reload = 0;
    
    if(page_replace_policy == FIFO)
    {
        //remove the first frame from the head of the FIFO queue
        frame_t *replace_page = (frame_t *)getmem(sizeof(frame_t));
        replace_page = fifo_head;
        
        if(fifo_head != NULL)
        {
            fifo_head = fifo_head->fifo;
            
            //remove the frame from backing store frm list
            bs_remove_frame(frm_tab[(replace_page->frame_num)-FRAME0].bs, replace_page->frame_num);
            
            //if the replacement frame belongs to the current process, then we need to flush thr TLB. Hence set the flag.
            if(frm_map[(replace_page->frame_num)-FRAME0].fr_pid == currpid)
                tlb_reload = 1;
            
            //free the frame
            free_frm((replace_page->frame_num)-FRAME0);
            
            //if the TLB needs to be flushed
            if(tlb_reload)
                write_cr3(proctab[currpid].pdbr);
            
            restore(ps);
            return(replace_page->frame_num);
        }
        else
        {
            restore(ps);
            return(SYSERR);
        }
    }
    else if(page_replace_policy == LRU)
    {
        frame_t *old = fifo_head;
        int min_age = -1;
        int frame_num = SYSERR;
        
        while(old != NULL)
        {
            //reduce age by half
            old->age = old->age >> 1;
            
            int flag = 0, shared_flag = 0;
            fr_map_t *next = &(frm_map[(old->frame_num)-FRAME0]);
            int fr_vpno = next->fr_vpno;
            int fr_pid = next->fr_pid;
            do
            {
                unsigned long int pd_offset = fr_vpno >>10;
                unsigned long int pt_offset = fr_vpno;
                pt_offset = pt_offset & 1023;
                
                //get the page table frame
                pd_t *pt_frame = (pd_t *)(proctab[fr_pid].pdbr);
                pt_frame += pd_offset;
                
                unsigned int pg_table = pt_frame->pd_base;
                
                pt_t *ptr = (pt_t *)(pg_table*NBPG);
                ptr += pt_offset;
                
                //now read the access bit
                if(ptr->pt_acc)
                {
                    if(!shared_flag)
                        old->age |= 128;
                    
                    old->age = old->age & 255;
                    
                    ptr->pt_acc = 0;
                    shared_flag = 1;
                }
                
                if(next->shared == NULL)
                    flag = 1;
                else
                {
                    next = next->shared;
                    fr_vpno = next->fr_vpno;
                    fr_pid = next->fr_pid;
                }
            } while(!flag);
            
            if(old->age < min_age)
            {
                min_age = old->age;
                frame_num = old->frame_num;
            }
            
            old = old->fifo;
        }
        
        //remove the frame from backing store frm list
        bs_remove_frame(frm_tab[frame_num-FRAME0].bs, frame_num);
        
        //if the replacement frame belongs to the current process, then we need to flush thr TLB. Hence set the flag.
        if(frm_map[frame_num-FRAME0].fr_pid == currpid)
            tlb_reload = 1;
        
        //free the frame
        free_frm(frame_num-FRAME0);
        
        if(tlb_reload)
            write_cr3(proctab[currpid].pdbr);
        
        restore(ps);
        return(frame_num);
    }
    restore(ps);
    return(SYSERR);
}


