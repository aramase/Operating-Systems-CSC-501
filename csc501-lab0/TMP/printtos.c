#include<stdio.h>

static unsigned long *ebp;

int a;
void printtos()
{
asm("movl %ebp,ebp");

kprintf("\nFunction [printtos()]\n");
kprintf("\nBefore \t\t[0x%08x]:\t 0x%08x", ebp+2,*(ebp+2));
kprintf("\nAfter \t\t[0x%08x]:\t 0x%08x", ebp,*(ebp));

//next 6 stack entries
for(a=1;a<=6;a++)
{
kprintf("\nStack Entry [%d]\t[0x%08x]:\t 0x%08x",a, ebp-(a),*(ebp-(a)));
}//end of for
}