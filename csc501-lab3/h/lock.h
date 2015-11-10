#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS	50
#endif

#define isbadlock(lock) (lock<0 || lock>=NLOCKS)

#define READ     1
#define WRITE    2

#define	LFREE	'\01' 
#define	LUSED	'\02'


struct lockentry {
	char	lockstate;  //current state of the lock
	int	    locknum;    //lock number
	int	    lockqhead;
	int 	lockqtail;
	int	    nreaders;   //readers
	int 	nwriters;   //writers associated with lock
};

extern struct lockentry locks[];
extern int nextlock;
extern int lockholdtab[][NLOCKS];

void linit();
int lcreate();
int ldelete(int lockdescriptor);
int lock(int ldes1, int type, int priority);
int releaseall(int numlocks, int ldes1, ...);
int obtainlock(int lock);
int release(int pid, int ldes);
#endif
