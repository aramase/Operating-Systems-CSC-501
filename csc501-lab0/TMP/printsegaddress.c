#include<stdio.h>

extern int etext, edata, end;

//To print etext,edata and end
void printsegaddress()
{
kprintf("\nFunction [printsegaddress()]\n");
kprintf("\nCurrent etext\t[0x%08x]:\t 0x%08x",&etext,etext);
kprintf("\nCurrent edata\t[0x%08x]:\t 0x%08x",&edata,edata);
kprintf("\nCurrent end\t[0x%08x]:\t 0x%08x",&end,end);
kprintf("\nPreceding etext\t[0x%08x]:\t 0x%08x",(&etext-1),*(&etext-1));
kprintf("\nPreceding edata\t[0x%08x]:\t 0x%08x",(&edata-1),*(&edata-1));
kprintf("\nPreceding end\t[0x%08x]:\t 0x%08x",(&end-1),*(&end-1));
kprintf("\nAfter etext\t[0x%08x]:\t 0x%08x",(&etext+1),*(&etext+1));
kprintf("\nAfter edata\t[0x%08x]:\t 0x%08x",(&edata+1),*(&edata+1));
kprintf("\nAfter end\t[0x%08x]:\t 0x%08x",(&end+1),*(&end+1));

}