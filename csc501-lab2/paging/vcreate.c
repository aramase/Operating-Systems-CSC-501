/* vcreate.c - vcreate */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
 static unsigned long esp;
 */

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
int	*procaddr;		/* procedure address		*/
int	ssize;			/* stack size in words		*/
int	hsize;			/* virtual heap size in pages	*/
int	priority;		/* process priority > 0		*/
char	*name;			/* name (for debugging)		*/
int	nargs;			/* number of args that follow	*/
long	args;			/* arguments (treated like an	*/
/* array in the code)		*/
{
    //kprintf("To be implemented!\n");
    
    STATWORD ps;
    disable(ps);
    //get a free backing store
    bsd_t bs_vacant = get_free_bs();
    
    //get pid
    int pid = create(procaddr, ssize, priority, name, nargs, args);
    
    //map the backing store
    bs_add_mapping(bs_vacant, pid, 4096, hsize);
    bs_tab[bs_vacant].bs_used_heap = 1;
    
    //update proctab fields
    proctab[pid].store = bs_vacant;
    proctab[pid].vhpno = bs_tab[bs_vacant].starting_page;
    proctab[pid].vhpnpages = hsize;
    proctab[pid].freememlist->vaddr = 4096*NBPG;
    proctab[pid].freememlist->vlen = hsize*NBPG;
    proctab[pid].freememlist->next = NULL;
    proctab[pid].freememlist->num = 1;
 

    restore(ps);
    return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
    int	pid;			/* process id to return		*/
    int	i;
    
    for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
        if ( (pid=nextproc--) <= 0)
            nextproc = NPROC-1;
        if (proctab[pid].pstate == PRFREE)
            return(pid);
    }
    return(SYSERR);
}
