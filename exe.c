#include "vtypes.h"
#include "coders.h"
#include "vos.h"

//signal(SIGINT,sig_done); #include <signal.h>

char szDescription[512]; // Default Description

int NeedReport() {
static time_t LastReport;
if (LastReport==TimeNow) return 0;
LastReport=TimeNow;
return 1;
}


void Reportf(uchar *fmt,...) {
#ifdef WIN32
uchar buf[1024];
BUF_FMT(buf,fmt);
win2dos(buf,buf,-1);
SetConsoleTitle(buf);
#endif
}

void sig_done(int sig) {
aborted = 1;

//printf("Abort signal!\n");
}

time_t TimeNow;
int aborted;
int sleepTime = 1;

void RunSleep0(int cnt) {
static time_t LastRun;
//printf("RunSleep=%d\n",cnt);
if (cnt) { LastRun=TimeNow; return ;} // Flash Them
if (LastRun+1<TimeNow) { // two seconds of silence???
    //printf("msleep, lastRun=%d, TimeNow=%d\n",(int)LastRun,(int)TimeNow);
    msleep(50); // Two Seconds of silence ???
    } else if (sleepTime) msleep(sleepTime); // small silence
//printf("NoCnt... LastRun=%d
}


void RunSleep(int cnt) {
static int LastTick;
int t = os_ticks();
if (cnt) { LastTick = t; return ;} // Flash Them
if (sleepTime && LastTick+sleepTime<t) { // two seconds of silence???
    msleep(sleepTime);
    }
}
