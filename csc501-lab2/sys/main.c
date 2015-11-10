
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
#define TPASSED 1
#define TFAILED 0

#define MYVADDR1	    0x40000000
#define MYVPNO1      0x40000
#define MYVADDR2     0x80000000
#define MYVPNO2      0x80000
#define MYBS1	1
#define MAX_BSTORE 16

#ifndef NBPG
#define NBPG 4096
#endif

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

void test1()
{
	char *addr0 = (char *)0x410000; // page 1040
	char *addr1 = (char*)0x40000000; //1G	
	

	kprintf("\nTest 1: Testing xmmap.\n");
	int  i= ((unsigned long)addr1)>>12;	

	*(addr0) = 'D';
	get_bs(MYBS1, 100);

	if (xmmap(i, MYBS1, 100) == SYSERR) {
	    kprintf("xmmap call failed\n");
	    return;
	}

	for (i=0; i<16; i++){
	    *addr1 = 'A'+i;
	    addr1 += NBPG;	//increment by one page each time
	}

	addr1 = (char*)0x40000000; //1G
	for (i=0; i<16; i++){
		kprintf("%c  ", *addr1);
		addr1 += NBPG;       //increment by one page each time
	}

	xmunmap(0x40000000>>12);
	release_bs(MYBS1);
	return;
}
/*----------------------------------------------------------------*/
void proc_test2(int i,int j,int* ret,int s) {
	char *addr;
	int bsize;
	int r;

	bsize = get_bs(i, j);
	if (bsize != 50)
		*ret = TFAILED;

	r = xmmap(MYVPNO1, i, j);
	if (j<=50 && r == SYSERR){
		*ret = TFAILED;
	}

	sleep(s);
	if (r != SYSERR) xmunmap(MYVPNO1);
	release_bs(i);
	return;
}

void test2() {
	int pids[16];
	int mypid;
	int i,j;

	int ret = TPASSED;
	kprintf("\nTest 2: Testing backing store operations\n");

	mypid = create(proc_test2, 2000, 20, "proc_test2", 4, 1,50,&ret,4);
	resume(mypid);

	sleep(5);
	kill(mypid);

	if (ret != TPASSED)
		kprintf("\tFAILED!\n");
	else
		kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test3(int i,int* ret) {
	char *addr;
	int bsize;
	int r;

	get_bs(i, 100);

	if (xmmap(MYVPNO1, i, 100) == SYSERR) {
	    *ret = TFAILED;
	    return;
	}
	sleep(4);
	xmunmap(MYVPNO1);
	release_bs(i);
	return;
}
void proc2_test3() {

	/*do nothing*/
	sleep(1);
	return;
}

void test3() {
	int pids[16];
	int mypid;
	int i,j;

	int ret = TPASSED;
	kprintf("\nTest 3: Private heap\n");


	for(i=0;i<=15;i++){
		pids[i] = create(proc1_test3, 2000, 20, "proc1_test3", 2, i,&ret);
		if (pids[i] == SYSERR){
			ret = TFAILED;
		}else{
			resume(pids[i]);
		}
	}
	sleep(5);
	mypid = vcreate(proc2_test3, 2000, 100, 20, "proc2_test3", 0, NULL);
	if (mypid != SYSERR)
		ret = TFAILED;

	for(i=0;i<=15;i++){
		kill(pids[i]);
	}
	if (ret != TPASSED)
		kprintf("\tFAILED!\n");
	else
		kprintf("\tPASSED!\n");
}

/*-------------------------------------------------------------------------------------*/
void proc1_test4(int* ret) {
	int *x;

	x = vgetmem(1024);
	if (x == NULL)
		*ret = TFAILED;

	*x = 100;
	*(x + 1) = 200;

	kprintf("*x = %d,  *(x + 1) = %d\n", *x, *(x+1));
	vfreemem(x, 1024);

	return;
}

void test4() {
	int pid1;
	int ret = TPASSED;

	kprintf("\nTest 4: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test4, 2000, 100, 20, "proc1_test4", 1, &ret);

	resume(pid1);
	sleep(3);
	kill(pid1);
}

/*-------------------------------------------------------------------------------------*/


void proc1_test5(int *ret) {

	char *vaddr, *addr0, *addr_lastframe, *addr_last;
	int i, j;
	int tempaddr;
    int addrs[1200];

	int vaddr_beg = 0x40000000;//1GB or page 262144
	int vpno;
	
	for(i = 0; i < MAX_BSTORE; i++){
		tempaddr = vaddr_beg + 100 * NBPG * i; 
		vaddr = (char *) tempaddr;
		vpno = tempaddr >> 12;
		get_bs(i, 100);
		if (xmmap(vpno, i, 100) == SYSERR) {
			*ret = TFAILED;
			kprintf("xmmap call failed\n");
			sleep(3);
			return;
		}

		for (j = 0; j < 100; j++) {
			*(vaddr + j * NBPG) = 'A' + i;
		}
        
		for (j = 0; j < 100; j++) {
			if (*(vaddr + j * NBPG) != 'A'+i){
				*ret = TFAILED;
				break;
			}
		}        
		xmunmap(vpno);
		release_bs(i);        
	}

	return;
}

void test5(){
	int pid1;
	int ret = TPASSED;
	kprintf("\nTest 5: Load test1\n");
	kprintf("1\n");

	pid1 = create(proc1_test5, 2000, 50, "proc1_test5",1,&ret);
	kprintf("2\n");

	resume(pid1);
		kprintf("3\n");

	sleep(4);
		kprintf("4\n");

	kill(pid1);
		kprintf("5\n");
	kprintf("ret: %d",ret);

	if (ret != TPASSED)
		kprintf("\tFAILED!\n");
	else
		kprintf("\tPASSED!\n");
}

void proc1_test6(int* ret) {
	int *x;
	int i;

	x = vgetmem(1024);
	if (x == NULL)
		*ret = TFAILED;

	for(i=0;i<20;i++)
		*(x + i) = (i+1)*100;

	for(i=0;i<20;i++)
		if( *(x + i) != (i+1)*100 ){
			*ret = TFAILED;
			break;
		}

	vfreemem(x, 1024);

	return;
}

void test6() {
	int pid1, pid2;
	int ret = TPASSED;
	int i;
	int pids[16];

	kprintf("\nTest 6: Load test2\n");
	for(i=0;i<=10;i++){
		pids[i] = vcreate(proc1_test6, 2000, 100, 20, "proc1_test6", 1, &ret);
		resume(pids[i]);
	}

	sleep(3);
	for(i=0;i<=10;i++){
		kill(pids[i]);
	}
	if (ret != TPASSED)
		kprintf("\tFAILED!\n");
	else
		kprintf("\tPASSED!\n");
}

/*-------------------------------------------------------------------------------------*/

void proc1_test7(char *msg, int lck, int* ret) {
  char *x;
  char temp;
  int i;

  get_bs(15, 100);
  if( xmmap(7000, 15, 100)==SYSERR ){
	  *ret = TFAILED;
	  return;
  }
  x = 7000*4096;

  for (i = 0; i < 26; i++) {
	  *(x + i * NBPG) = 'A' + i;
  }

  xmunmap(7000);
  return;
}

void proc2_test7(char *msg, int lck, int* ret){
  char *x;
  char temp_b;
  int i;

  get_bs(15, 100);
  if( xmmap(6000, 15, 100)==SYSERR ){
	  *ret = TFAILED;
	  return;
  }
  x = 6000 * 4096;

  for (i = 0; i < 26; i++) {
	  kprintf("%c  ", *(x + i * NBPG));
  }

  xmunmap(6000);
  release_bs(15);
  return;
}

void test7() {
	int pid1;
	int pid2;
	int ret = TPASSED;
	kprintf("\nTest 7: Shared backing store\n");

	pid1 = create(proc1_test7, 2000, 20, "proc1_test7", 1, &ret);
	pid2 = create(proc2_test7, 2000, 20, "proc2_test7", 1, &ret);

	resume(pid1);
	sleep(3);
	resume(pid2);

	sleep(3);
	kill(pid1);
	kill(pid2);

}


int main() {
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();

	return 0;
}
