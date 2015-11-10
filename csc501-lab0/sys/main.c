/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include<lab0.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
kprintf("\n\nHello World, Xinu lives\n\n");

long param=0xaabbccdd;
kprintf("Input: 0x%08x\nOutput: 0x%08x\n", param, zfunction(param));	
kprintf("\n");

printsegaddress();
kprintf("\n");

printtos();
kprintf("\n");

printprocstks(30);

syscallsummary_start();
sleep(15);
wait(5);
sleep(10);
syscallsummary_stop();
kprintf("\n");

printsyscalls();

//To check Endianness
/*short int a=0x002a;
char *p=(char *)(&a);
if(*p==0x00)
kprintf("\nBig Endian");
else
kprintf("\nLittle Endian");
return 0;*/
}
