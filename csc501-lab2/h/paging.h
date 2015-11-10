/* paging.h */

#ifndef _PAGING_H_
#define _PAGING_H_

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {
    
    unsigned int pd_pres: 1;		/* page table present?		*/
    unsigned int pd_write : 1;		/* page is writable?		*/
    unsigned int pd_user: 1;		/* is use level protection?	*/
    unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
    unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
    unsigned int pd_acc	: 1;		/* page table was accessed?	*/
    unsigned int pd_mbz	: 1;		/* must be zero			*/
    unsigned int pd_fmb	: 1;		/* four MB pages?		*/
    unsigned int pd_global: 1;		/* global (ignored)		*/
    unsigned int pd_avail : 3;		/* for programmer's use		*/
    unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {
    
    unsigned int pt_pres: 1;		/* page is present?		*/
    unsigned int pt_write: 1;		/* page is writable?		*/
    unsigned int pt_user: 1;		/* is use level protection?	*/
    unsigned int pt_pwt	: 1;		/* write through for this page? */
    unsigned int pt_pcd	: 1;		/* cache disable for this page? */
    unsigned int pt_acc	: 1;		/* page was accessed?		*/
    unsigned int pt_dirty : 1;		/* page was written?		*/
    unsigned int pt_mbz	: 1;		/* must be zero			*/
    unsigned int pt_global: 1;		/* should be zero in 586	*/
    unsigned int pt_avail : 3;		/* for programmer's use		*/
    unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
    unsigned int pg_offset : 12;		/* page offset			*/
    unsigned int pt_offset : 10;		/* page table offset		*/
    unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct _frame_t {
    int status;
    int refcnt;
    bsd_t bs;
    int bs_page;
    struct _frame_t *bs_next;
    struct _frame_t *fifo;
    unsigned int age;
    int frame_num;
} frame_t;

typedef struct _bs_map_t {
    struct _bs_map_t *next;
    bsd_t bs;
    int bs_pid;                        //process mapped to backing store
    int bs_vpno;
    int bs_npages;
    int starting_page;
    int bs_sem;
} bs_map_t;

typedef struct {
    int bs_status;
    int bs_used_heap; /* is this bs used by heap?*/
    int bs_npages; /* number of pages in the store */
    bs_map_t *map_pid; /* where it is mapped*/
    frame_t *frm; /* the list of frames that maps this bs*/
    int starting_page;
} bs_t;

typedef struct _fr_map_t {
    int fr_status;			/* MAPPED or UNMAPPED		*/
    int fr_pid;				/* process id using this frame  */
    int fr_vpno;				/* corresponding virtual page no*/
    int fr_refcnt;			/* reference count number of things pointing to your page*/
    int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
    int fr_dirty;
    struct _fr_map_t *shared;				/* shared frames	*/
    int frame_num;
    int bs_page_num;
}fr_map_t;

typedef struct _virtmemblock{
    unsigned long vaddr;                //virtual address
    unsigned long vlen;                 //virtual length
    unsigned int num;
    struct _virtmemblock *next;
}virtmemblock;

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

#define NFRAMES 	1024	/* number of frames		*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED      1

#define FRM_UNMAPPED	0
#define FRM_MAPPED      1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define FRM_FREE    0
#define FRM_PGD     1
#define FRM_PGT     2
#define FRM_BS      5

#define FIFO		3
#define LRU         4

#define MAX_ID          15              /* You get 16 mappings, 0 - 15 */

#define BACKING_STORE_BASE      0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define NBS      16
#define BS_PAGES 2048


extern bs_map_t bs_map[NBS];
extern fr_map_t frm_map[NFRAMES];
extern bs_t bs_tab[NBS];
extern frame_t frm_tab[NFRAMES];
extern frame_t *fifo_head, *fifo_tail;//for page replacement
extern int page_table[((unsigned long) BACKING_STORE_UNIT_SIZE)*NBS/NBPG];

/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xmunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(int src_frame, int dst_page);



#define FP2FN(frm)  (((frm) - frm_map) + FRAME0)
#define FN2ID(fn)   ((fn) - FRAME0)
#define FP2PA(frm)  ((void*)(FP2FN(frm) * NBPG))

#define AVAILABILITY    ((unsigned long) BACKING_STORE_UNIT_SIZE)*NBS/NBPG
#define PT_SIZE         ((unsigned long) BACKING_STORE_UNIT_SIZE)*NBS/NBPG
#define BACKING_SIZE    ((unsigned long) BACKING_STORE_UNIT_SIZE)/NBPG


extern int total_bs_available;
extern int page_replace_policy;
extern int bs_size;


#endif