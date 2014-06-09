/*

  Linux vos implementation
  thread->pthread

*/
#include "vos.h"

#define __USE_UNIX98
#include "pthread.h"
#include "stdlib.h"  // malloc,free
#include "unistd.h"  // usleep
#include <sys/time.h> // gettimeofday

void *thread_create(void *proc, void *arg) {
pthread_t res = 0;
int err = pthread_create(&res,0,(void*)proc,arg);
if (err) return 0;
return (void*)res;
}

void thread_destory(void *thread){
pthread_t t = (pthread_t)thread;
pthread_detach(t);
pthread_cancel(t);
}

// mutex
void *mutex_create() { // reenter mutex
pthread_mutex_t *cs;
cs = malloc(sizeof(pthread_mutex_t));
pthread_mutexattr_t    attrs;
pthread_mutexattr_init(&attrs);
pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init( cs, &attrs );
return cs;
}

void mutex_lock(void *mutex) {
if (mutex) pthread_mutex_lock(mutex);
}

void mutex_unlock(void *mutex) {
if (mutex) pthread_mutex_unlock(mutex);
}

void mutex_destroy(void * mutex) {
if (!mutex) return ;
free(mutex); // free mem or do nothing?
}


// timers

void msleep(int msec) { usleep(1000*msec); }

unsigned long os_ticks() { //GetTickCount()
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec*1000+tv.tv_usec/1000);
}


// exec external program
#include <stdio.h>

int os_exec(char *cmd, int (*on_out)(), void *arg) { // on_out ret 0 = ok, continue
FILE *f = popen(cmd,"w");
if (!f) return 0; // error on start
char buf[1024];
if (f) while(1) {
     int bytes = fread(buf,sizeof(buf),1,f);
     if (bytes<=0) break;
     bytes = on_out(arg,buf,bytes); // print or other
     if (bytes!=0) { pclose(f); return bytes; }
     }
pclose(f);
return 1; // ok
}


#include <sys/resource.h>
//#include <procfs.h>
int os_mem_used() { // long int ???
    struct rusage rusage;
    getrusage( RUSAGE_SELF, &rusage );
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t)(rusage.ru_maxrss * 1024L);
#endif


}

