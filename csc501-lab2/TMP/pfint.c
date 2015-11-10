/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
    STATWORD ps;
    disable(ps);
    unsigned long int eip = read_cr2();
    
    unsigned long int pd_offset = eip >> 22;
    unsigned long int pt_offset = eip >>12;
    pt_offset = pt_offset & 1023;
    unsigned long offset = eip;
    offset = offset & 4095;
    
    eip = eip >> 12; //vpno
    
    int i = 0, flag = 0;
    int backing_store_page, bs_id;
    
    for(i = 0 ; i < NBS; i++)
    {
        if(proctab[currpid].map[i].bs_pid == currpid)
        {
            if(proctab[currpid].map[i].next == NULL)
            {
                if(proctab[currpid].map[i].bs_vpno == eip) //we found the exact page
                {
                    backing_store_page = proctab[currpid].map[i].starting_page;
                    flag = 1;
                }
                else if(proctab[currpid].map[i].bs_vpno < eip && (proctab[currpid].map[i].bs_vpno+proctab[currpid].map[i].bs_npages) >= eip)
                {
                    backing_store_page = proctab[currpid].map[i].starting_page + eip - proctab[currpid].map[i].bs_vpno;
                    flag = 1;
                }
            }
            else
            {
                bs_map_t *move = &(proctab[currpid].map[i]);
                while(move != NULL)
                {
                    if(move->bs_vpno == eip) //we found the exact page
                    {
                        backing_store_page = move->starting_page;
                        flag = 1;
                        break;
                    }
                    else if(move->bs_vpno < eip && (move->bs_npages+move->bs_vpno) >= eip)
                    {
                        backing_store_page = move->starting_page + eip - move->bs_vpno;
                        flag = 1;
                        break;
                    }
                    move = move->next;
                }
            }
        }
        
        if(flag==1)
        {
            bs_id = i;
            break;
        }
        
    }
    
    unsigned long *bs_addr = (unsigned long *)(backing_store_page*NBPG);
    
    //populate page table
    //checking if page dir is empty
    
    pd_t *ptr1 = (pd_t *)(proctab[currpid].pdbr);
    ptr1 += pd_offset;
    pt_t *ptr;
    if(ptr1->pd_pres == 1) //page table exists hence add entry to that
    {
        ptr = (pt_t *)((ptr1->pd_base)*NBPG);
        frm_tab[(ptr1->pd_base)-FRAME0].refcnt++;
        frm_map[(ptr1->pd_base)-FRAME0].fr_refcnt++;
    }
    else //we need to create a page table, add our free_frame entry to it and add the page table entry to the directory
    {
        //kprintf("\nin else %d\n",ptr1);
        
        int pt_frame = get_frm();
        frm_tab[pt_frame-FRAME0].status = FRM_PGT;
        frm_tab[pt_frame-FRAME0].refcnt++;
        
        frm_map[pt_frame-FRAME0].fr_status = FRM_MAPPED;
        frm_map[pt_frame-FRAME0].fr_pid = currpid;
        frm_map[pt_frame-FRAME0].fr_refcnt++;
        frm_map[pt_frame-FRAME0].fr_type = FR_TBL;
        
        ptr = (pt_t*)(pt_frame*NBPG);
        
        //add the above table to page directory
        ptr1->pd_pres = 1;
        ptr1->pd_write = 1;
        ptr1->pd_user = 0;
        ptr1->pd_pwt = 0;
        ptr1->pd_pcd = 0;
        ptr1->pd_acc = 0;
        ptr1->pd_mbz = 0;
        ptr1->pd_fmb = 0;
        ptr1->pd_global = 0;
        ptr1->pd_avail = 0;
        ptr1->pd_base = pt_frame;
        
    }
    //add entry to page table
    ptr += pt_offset;
    ptr->pt_pres = 1;
    ptr->pt_write = 1;
    ptr->pt_user = 0;
    ptr->pt_pwt = 0;
    ptr->pt_pcd = 0;
    ptr->pt_acc = 0;
    ptr->pt_dirty = 0;
    ptr->pt_mbz = 0;
    ptr->pt_global = 0;
    ptr->pt_avail = 0;
    
    //getting a free frame an setting the frame mappings
    int free_frame;
    if(page_table[backing_store_page-total_bs_available] != -1)
    {
        free_frame = page_table[backing_store_page-total_bs_available];
        frm_map[free_frame-FRAME0].fr_refcnt++;
        frm_tab[free_frame-FRAME0].refcnt++;
        
        fr_map_t *map = (fr_map_t *)getmem(sizeof(fr_map_t));
        map->fr_pid = currpid;
        map->fr_vpno = eip;
        map->shared = NULL;
        
        fr_map_t *next = (fr_map_t *)getmem(sizeof(fr_map_t));
        next = &(frm_map[free_frame-FRAME0]);
        
        while(next->shared != NULL)
            next = next->shared;
        
        next->shared = map;

    }
    else
    {
        //obtain free frame
        free_frame = get_frm();
        
        frm_tab[free_frame-FRAME0].status = FRM_BS;
        frm_tab[free_frame-FRAME0].refcnt++;
        frm_tab[free_frame-FRAME0].bs = bs_id;
        frm_tab[free_frame-FRAME0].bs_page = backing_store_page;
        frm_tab[free_frame-FRAME0].bs_next = NULL;
        frm_tab[free_frame-FRAME0].fifo = NULL;
        frm_tab[free_frame-FRAME0].age = 128;
        frm_map[free_frame-FRAME0].fr_status = FRM_MAPPED;
        frm_map[free_frame-FRAME0].fr_pid = currpid;
        frm_map[free_frame-FRAME0].fr_vpno = eip;
        frm_map[free_frame-FRAME0].fr_refcnt++;
        frm_map[free_frame-FRAME0].fr_type = FR_PAGE;
        frm_map[free_frame-FRAME0].bs_page_num = backing_store_page;
        
        page_table[backing_store_page-total_bs_available] = free_frame;
        
        //set bs mappings
        if(bs_tab[bs_id].frm == NULL)
            bs_tab[bs_id].frm = &frm_tab[free_frame-FRAME0];
        else
        {
            frame_t *move = (frame_t *)getmem(sizeof(frame_t));
            move = bs_tab[bs_id].frm;
            while(move->bs_next != NULL)
            {
                move = move->bs_next;
            }
            
            move->bs_next = &frm_tab[free_frame-FRAME0];
        }
        
        //adding this frame to the fifo queue
        if(fifo_head == NULL)
        {
            //queue is empty
            fifo_head = &frm_tab[free_frame-FRAME0];
            fifo_tail = fifo_head;
        }
        else
        {
            fifo_tail->fifo = &frm_tab[free_frame-FRAME0];
            fifo_tail = &frm_tab[free_frame-FRAME0];
        }
    }
    
    unsigned long *dest_addr = (unsigned long *)(free_frame*NBPG);
    
    //copy page from bs to phy
    for(i = 0; i < NBPG/sizeof(unsigned long); i++)
    {
        *dest_addr = *bs_addr;
        dest_addr++;
        bs_addr++;
        
    }
    
    ptr->pt_base = free_frame;
    
    restore(ps);
    return OK;
}