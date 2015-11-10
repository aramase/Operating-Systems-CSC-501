/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include<lab0.h>
#include<proc.h>

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{

//changes for incrementing count on syscall
int start_time;
if(call_active == 1) 
{
call_used[currpid] = 1;
call_frequency[currpid][1]++;//id for gettime()=1
start_time = ctr1000;
}

if(call_active == 1) 
{
call_duration[currpid][1] += ctr1000 - start_time;
}

return OK;
}
