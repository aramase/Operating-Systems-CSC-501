#define MULTIQSCHED 1
#define LINUXSCHED  2


extern int  getschedclass();
extern void setschedclass(int);
extern int  scheduleclass;
extern int container;
extern int current_queue_flag;
