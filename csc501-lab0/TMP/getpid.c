/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include<lab0.h>

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{

//changes for incrementing count on syscall
int start_time;
if(call_active == 1) 
{
call_used[currpid] = 1;//set the flag for process
call_frequency[currpid][0]++;//id for getpid()=0
start_time = ctr1000;
}

//to calculate duration of syscall
if(call_active == 1) 
{
call_duration[currpid][0] += ctr1000 - start_time;
}
return(currpid);
}
