#include "logger.h"
#include <time.h>
#include <stdarg.h>
#include "vos.h"

int log2stdout = 5; // max log level to out on a screen
logger *lastLogOpen = 0;

void loggerDone(logger *f) {
if (f->file) { fclose(f->file); f->file=0;}
}

VS0OBJ0(logger,loggerDone);

logger *logOpen(uchar *filename) { // Тупо - пока так - дефолтовый логгер
FILE *f; logger *log;
f = fopen(filename,"ab");
//if (!f) f = fopen(filename,"b");
if (!f) return 0;
log = loggerNew();
log->mutex = mutex_create();
log->file = f;
lastLogOpen = log;
return log;
}

void logClose(logger *log) { // Тупо - пока так - дефолтовый логгер
if (log->file) {
    logWrite(log,"log",0,"logClosed");
    fclose(log->file);
    log->file=0;
    }
}

//#define BUF_FMT(buf,fmt) {va_list va; va_start(va,fmt); _vsnprintf(buf,sizeof(buf)-1,fmt,va); va_end(va);}

char szTimeNow[2*18],szTime2Now[2*18],szTime3Now[2*18];
struct tm sysTime; // myTime

void TimeUpdate() {
static time_t pTimeNow;
time(&TimeNow); // GlobalRef
if (pTimeNow!=TimeNow) {
    struct tm t;
    t=sysTime = *localtime(&TimeNow);
    sprintf(szTimeNow,"%02d-%02d-%02d %02d:%02d:%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    sprintf(szTimeNow,"%02d-%02d-%02dT%02d:%02d:%02d",t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    sprintf(szTime2Now,"%04d-%02d-%02d-%02d-%02d-%02d",2000+t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    sprintf(szTime3Now,"%04d-%02d-%02dT%02d:%02d:%02d",2000+t.tm_year%100,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    pTimeNow = TimeNow;
    if (lastLogOpen) logUpdateNow(lastLogOpen);
    }
}

void logUpdateNow(logger *log) {
if (!log || !log->file) return ;
TimeUpdate();
if (log->now!=TimeNow) {
    if (log->file) fflush(log->file); // Flush to disk
    }
}

void logWrite(logger *log,uchar *src,int lev,char *fmt,...) { // Сливаем в лог эту строчку - метим временем
int i; uchar *p;
if (!log) log = lastLogOpen;
if (!log || !log->file) { // just print on a screen
    uchar buf[128];
    BUF_FMT(buf,fmt);
    //buf[sizeof(buf)-1]=0;
    #ifdef WIN32
    win2dos(buf,buf,strlen(buf));
    #endif
    p = buf; for(i=0;p[i];i++); while(i>0 && p[i-1]<=32) i--; p[i]=0;
    printf("--%s\n",buf);
    return;
    }
mutex_lock(log->mutex);
logUpdateNow(log);
p=log->buf;
for(i=0;i<lev && i<10;i++) p[i]=' '; p[i]=0;
fprintf(log->file,"[%s] (%s) %s",szTime3Now,src,p); // Сначала сливаем строку - время
BUF_FMT(log->buf, fmt);
//while(*p && *p<=32) p++;
for(i=0;p[i];i++); while(i>0 && p[i-1]<=32) i--; p[i]=0;
fprintf(log->file,"%s\n",p);
fflush(log->file);
if (lev<=log2stdout) {
 #ifdef WIN32
 win2dos(p,p,i); // convert
 #endif
 printf("-->%s\n",p); // 2 display
 }
mutex_unlock(log->mutex);
}

void hexDump(FILE *file, uchar *data, int len) {
int i;
uchar buf[256];
if (len<=0) return ;
while(len>0) {
    uchar *p = buf;
    for(i=0;i<16;i++,p+=3) {
        if (i==8) { *p='|'; p++; } // Word Delimiter
        if (i<len) sprintf(p,"%02x ",data[i]); // All A Data
          else sprintf(p,"   "); // Filler
        }
    *p=' '; p++; *p = '|'; p++; *p=' '; p++;
    for(i=0;i<16 && i<len; i++,p++) *p=data[i]<32?'.':data[i];
    *p=0;
    fprintf(file,"%s\n",buf);
    data+=16; len-=16;
    }
}

void logDump(logger *log,uchar *src,int lev,void *data, int dlen, uchar *fmt,...) { // Сливаем в лог эту строчку - метим временем
int i; uchar *p;
if (!log || !log->file) return;
mutex_lock(log->mutex);
logUpdateNow(log);
p=log->buf;
for(i=0;i<lev && i<10;i++) p[i]=' '; p[i]=0;
fprintf(log->file,"[%s] (%s) %s",szTimeNow,src,p); // Сначала сливаем строку - время
BUF_FMT(log->buf, fmt);
//while(*p && *p<=32) p++;
for(i=0;p[i];i++); while(i>0 && p[i-1]<=32) i--; p[i]=0;
fprintf(log->file,"%s\n",p);
if (dlen>0) hexDump(log->file,data,dlen); // Dump a data
if (log2stdout) {
#ifdef WIN32
 win2dos(p,p,i); // convert - print on a screen???
#endif
 printf("-->%s\n",p); // 2 display
 if (dlen>0) hexDump(stdout,data,dlen); // Dump a data
 }
mutex_unlock(log->mutex);
}
