/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bs_map[NBS];
bs_t bs_tab[NBS];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */

int page_table[PT_SIZE];
int total_bs_available=AVAILABILITY;

SYSCALL init_bsm()
{
    STATWORD ps;
    disable(ps);
    
    int a;
    for(a = 0;a < NBS; a++)
    {
        bs_tab[a].bs_status = BSM_UNMAPPED;
        bs_tab[a].bs_used_heap = -1;
        bs_tab[a].bs_npages = -1;
        bs_tab[a].map_pid = NULL;
        bs_tab[a].frm = NULL;
        bs_tab[a].starting_page = BS_PAGES+(a*BACKING_SIZE);
    }
    
    //reinitialize page table entries
    for(a = 0;a < total_bs_available; a++)
    {
        page_table[a] = -1;
    }
    
    restore(ps);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL get_bsm(int* avail)
{
    STATWORD ps;
    disable(ps);
    
    int a;
    for(a = 0 ; a < NBS; a++)
        if(bs_tab[a].bs_status == BSM_UNMAPPED)
        {
            restore(ps);
            return a;//return the bs id
        }
    
    restore(ps);
    return(SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab
 *-------------------------------------------------------------------------
 */

SYSCALL free_bsm(int i)
{
    STATWORD ps;
    disable(ps);
    
    bs_tab[i].bs_status = BSM_UNMAPPED;
    bs_tab[i].bs_npages = 0;
    bs_tab[i].frm = NULL;
    bs_tab[i].bs_used_heap = 0;
    bs_tab[i].map_pid->next = NULL;
    bs_tab[i].map_pid->bs = -1;
    bs_tab[i].map_pid->bs_pid = -1;
    bs_tab[i].map_pid->bs_vpno = -1;
    bs_tab[i].map_pid->bs_npages = 0;
    
    restore(ps);
}

//initialize backing stores
int init_bs()
{
    init_bsm();
}

//get free backing store
bsd_t get_free_bs()
{
    STATWORD ps;
    disable(ps);
    
    int a;
    for(a = 0; a < NBS; a++)
    {
        if(bs_tab[a].bs_status == BSM_UNMAPPED)
        {
            restore(ps);
            return a;
        }
    }
    restore(ps);
    return(SYSERR);
}

//freeing the backing store
void free_bs(bsd_t id)
{
    STATWORD ps;
    disable(ps);
    
    bs_tab[id].bs_status = BSM_UNMAPPED;
    bs_tab[id].bs_npages = 0;
    bs_tab[id].frm = NULL;
    bs_tab[id].bs_used_heap = 0;
    bs_tab[id].map_pid->next = NULL;
    bs_tab[id].map_pid->bs = -1;
    bs_tab[id].map_pid->bs_pid = -1;
    bs_tab[id].map_pid->bs_vpno = -1;
    bs_tab[id].map_pid->bs_npages = 0;
    
    restore(ps);
    
}


//add a mapping of the bs to (pid, vpno)
void bs_add_mapping(bsd_t id, int pid, int vpno, int npages)
{
    //get the backing store for mapping
    get_bs(id,npages);
    
    proctab[pid].map[id].bs_vpno = vpno;
    proctab[pid].map[id].bs_npages = npages;
    proctab[pid].map[id].bs = id;
    proctab[pid].map[id].bs_pid = pid;
    proctab[pid].map[id].starting_page = BS_PAGES+(id*BACKING_SIZE);
    proctab[pid].map[id].next = NULL;
    
    if(bs_tab[id].map_pid == NULL)
    {
        bs_tab[id].map_pid = &(proctab[pid].map[id]);
    }
    else
    {
        bs_map_t *move = (bs_map_t *)getmem(sizeof(bs_map_t));
        move = bs_tab[id].map_pid;
        while(move->next != NULL)
            move = move->next;
        
        move->next = &(proctab[pid].map[id]);
    }
}


//remove a mapping from both the backstore and process
int bs_del_mapping(int pid, int vpno)
{
    vpno = bs_get_vpno(pid,vpno);
    
    //find out the backing store that contains this vpno from the map in proctab
    int a;
    for(a = 0; a < NBS; a++)
    {
        if(proctab[pid].map[a].bs_pid == pid && proctab[pid].map[a].bs_vpno == vpno)
        {
            bs_map_t *next = (bs_map_t *)getmem(sizeof(bs_map_t));
            next = bs_tab[a].map_pid;
            bs_map_t *prev = (bs_map_t *)getmem(sizeof(bs_map_t));
            
            if(next == &(proctab[pid].map[a]))
            {
                bs_tab[a].map_pid = NULL;
            }
            else
            {
                while(next != NULL)
                {
                    if(next == &(proctab[pid].map[a]))
                    {
                        prev->next = next->next;
                        break;
                    }
                    prev = next;
                    next = next->next;
                }
            }
            
            //Now resetting the map entries in proctab
            proctab[pid].map[a].next = NULL;
            proctab[pid].map[a].bs = -1;
            proctab[pid].map[a].bs_pid = -1;
            proctab[pid].map[a].bs_vpno = -1;
            proctab[pid].map[a].bs_npages = 0;
            
            //Now we traverse the frame linked list in bs_tab and keep returning those frames
            frame_t *next_frame = (frame_t *)getmem(sizeof(frame_t));
            next_frame = bs_tab[a].frm;
            frame_t *prev_frame = (frame_t *)getmem(sizeof(frame_t));
            
            prev_frame = next_frame;
            while(next_frame != NULL)
            {
                if(frm_map[(next_frame->frame_num)-FRAME0].shared == NULL)
                {
                    if(frm_map[(next_frame->frame_num)-FRAME0].fr_pid == pid)
                    {
                        free_frm((next_frame->frame_num)-FRAME0);
                    }
                }
                else
                {
                    frm_map[(next_frame->frame_num)-FRAME0].fr_refcnt--;
                    frm_tab[(next_frame->frame_num)-FRAME0].refcnt--;
                    if(frm_tab[(next_frame->frame_num)-FRAME0].refcnt <= 0)
                    {
                        free_frm((next_frame->frame_num)-FRAME0);
                    }
                    else
                    {
                        if(frm_map[(next_frame->frame_num)-FRAME0].fr_pid == pid)
                        {
                            frm_map[(next_frame->frame_num)-FRAME0].fr_pid = frm_map[(next_frame->frame_num)-FRAME0].shared->fr_pid;
                            frm_map[(next_frame->frame_num)-FRAME0].fr_vpno = frm_map[(next_frame->frame_num)-FRAME0].shared->fr_vpno;
                            frm_map[(next_frame->frame_num)-FRAME0].shared = frm_map[(next_frame->frame_num)-FRAME0].shared->shared;
                        }
                        else
                        {
                            fr_map_t *nextmapping = (fr_map_t *)getmem(sizeof(fr_map_t));
                            fr_map_t *prevmapping = (fr_map_t *)getmem(sizeof(fr_map_t));
                            nextmapping = &(frm_map[(next_frame->frame_num)-FRAME0]);
                            prevmapping = nextmapping;
                            
                            while(nextmapping != NULL)
                            {
                                if(nextmapping->fr_pid == pid)
                                    prevmapping->shared = nextmapping->shared;
                                
                                prevmapping = nextmapping;
                                nextmapping = nextmapping->shared;
                            }
                        }
                    }
                }
                
                prev_frame = next_frame;
                next_frame = next_frame->bs_next;
                prev_frame->bs_next = NULL;
            }
            
        }
    }
}


int bs_lookup_mapping(int pid, long vpno)
{
    STATWORD ps;
    disable(ps);
    
    int a;
    for(a = 0; a < NBS; a++)
        if(proctab[pid].map[a].bs_pid == pid && proctab[pid].map[a].bs_vpno == vpno)
        {
            restore(ps);
            return a;
        }
    
    restore(ps);
    return(SYSERR);
    
}


void bs_remove_frame(int bs_id, int frame_num)
{
    frame_t *next = (frame_t *)getmem(sizeof(frame_t));
    next = bs_tab[bs_id].frm;
    frame_t *prev = (frame_t *)getmem(sizeof(frame_t));
    
    prev = next;
    
    if(next->frame_num == frame_num)
        bs_tab[bs_id].frm = bs_tab[bs_id].frm->bs_next;
    else
    {
        while(next != NULL)
        {
            if(next->frame_num == frame_num)
            {
                prev->bs_next = next->bs_next;
            }
            prev = next;
            next = next->bs_next;
        }
    }
    
}


int bs_get_vpno(int pid, int vpno)
{
    STATWORD ps;
    int a, succcess_flag = 0;
    int return_vpno;
    
    disable(ps);
    
    for(a = 0 ; a < NBS; a++)
    {
        if(proctab[pid].map[a].bs_pid == pid)
        {
            if(proctab[pid].map[a].next == NULL)
            {
                if(proctab[pid].map[a].bs_vpno == vpno) //we found the exact vpno
                {
                    return_vpno = proctab[pid].map[a].bs_vpno;
                    succcess_flag = 1;
                }
                else if(proctab[pid].map[a].bs_vpno < vpno && (proctab[pid].map[a].bs_vpno+proctab[pid].map[a].bs_npages) >= vpno)
                {
                    return_vpno = proctab[pid].map[a].bs_vpno;
                    succcess_flag = 1;
                }
            }
            else
            {
                bs_map_t *move = &(proctab[pid].map[a]);
                while(move != NULL)
                {
                    if(move->bs_vpno == vpno)
                    {
                        return_vpno = move->bs_vpno;
                        succcess_flag = 1;
                        break;
                    }
                    else if(move->bs_vpno < vpno && (move->bs_npages+move->bs_vpno) >= vpno)
                    {
                        return_vpno = move->bs_vpno;
                        succcess_flag = 1;
                        break;
                    }
                    move = move->next;
                }
            }
        }
        
        if(succcess_flag==1)
            break;
    }
    
    restore(ps);
    return(return_vpno);
}

void bs_kill(int pid)
//realease the backing store when the proc is killed
//need to remove the entries mapped to it as well 
{
    int a;
    for(a = 0; a < NBS; a++)
    {
        if(proctab[pid].map[a].bs_pid == pid)
        {
            xmunmap(proctab[pid].map[a].bs_vpno);
            release_bs(a);
        }
    }
    
    //deleting processes page directory
    int page_dir = proctab[pid].pdbr/NBPG;
    
    frm_tab[page_dir-FRAME0].status = FRM_FREE;
    frm_tab[page_dir-FRAME0].refcnt = 0;
    frm_tab[page_dir-FRAME0].bs = -1;
    frm_tab[page_dir-FRAME0].bs_page = -1;
    frm_tab[page_dir-FRAME0].bs_next = NULL;
    frm_tab[page_dir-FRAME0].fifo = NULL;
    frm_tab[page_dir-FRAME0].age = 0;
    
    frm_map[page_dir-FRAME0].fr_status = FRM_UNMAPPED;
    frm_map[page_dir-FRAME0].fr_pid = -1;
    frm_map[page_dir-FRAME0].fr_vpno = -1;
    frm_map[page_dir-FRAME0].fr_refcnt = 0;
    frm_map[page_dir-FRAME0].fr_type = FR_PAGE;
    frm_map[page_dir-FRAME0].fr_dirty = -1;
    frm_map[page_dir-FRAME0].shared = NULL;
    frm_map[page_dir-FRAME0].bs_page_num = -1;
    
}