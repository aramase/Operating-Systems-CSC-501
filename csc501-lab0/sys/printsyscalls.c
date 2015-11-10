#include<stdio.h>
#include<kernel.h>
#include<proc.h>
#include<lab0.h>

int call_active=0;//to start the trace of system calls
int call_frequency[NPROC][6];//number of times the system call is invoked
int call_duration[NPROC][6];//total duration of system call
int call_used[NPROC];//to check if any system call was invoked by process

//defining the names of system calls
const char call_name[6][16]={"getpid()","gettime()","kill()","signal()","sleep()","wait()"};


//for starting the trace
void syscallsummary_start()
{
call_active = 1;

//initialization of variables
int a,b;
for(a = 0; a < NPROC; ++a) 
{
call_used[a] = 0;
for(b = 0; b < 6; ++b)
{
call_frequency[a][b] = 0;
call_duration[a][b] = 0;
}
}
}

//stop the trace
void syscallsummary_stop()
{
call_active = 0;
}

//printing the details of system calls
void printsyscalls()
{

kprintf("\nFunction [printsyscalls()]\n");
int a,b;
for(a = 0; a < NPROC; ++a)
{
if( call_used[a] == 0 )//has not invoked any system calls
{
continue;
}
else 
{
kprintf("\nProcess [pid:%d]\n", a);
for(b = 0; b < 6; ++b) 
{                
if(call_frequency[a][b] > 0) 
{		    
kprintf("\nSystem Call: %s, \nNo. of times [%s] invoked: %d, \nDuration: %d ms\nAvg Execution time: %d ms", call_name[b], call_name[b], call_frequency[a][b], call_duration[a][b],call_duration[a][b]/call_frequency[a][b]);
}//end of if
}//end of for
}//end of else
}//end of for(a)
}
