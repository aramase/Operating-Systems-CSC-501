#include<stdio.h>
#include<kernel.h>
#include<proc.h>

static unsigned long *esp;

void printprocstks(int priority)
{
int a;
struct pentry *proc;//New structure proc

kprintf("\nFunction [printprocstks(%d)]\n",priority);

for(a=0;a<NPROC;++a)
{
proc=&proctab[a];

if((proc->pstate)==PRFREE)//if the process is free
{
continue;
}

else if (proc->pprio<=priority)//less than priority
{
kprintf("\nPROCESS:\t %s",proc->pname);
kprintf("\nProc ID:\t[%d]",a);
kprintf("\nPriority:\t %d",proc->pprio);
kprintf("\nSTACK BASE:\t 0x%08x",proc->pbase);
kprintf("\nSTACK SIZE:\t %d",proc->pstklen);
kprintf("\nSTACK LIMIT:\t 0x%08x",proc->plimit);

if(a==currpid)//for current running process
{
asm("movl %esp,esp");
kprintf("\nSTACK POINTER:\t 0x%08x",esp);
}
else
kprintf("\nSTACK POINTER:\t 0x%08x",proc->pesp);
}//end of else if
kprintf("\n");
}//end of for
}
