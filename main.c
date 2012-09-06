#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vos.h"

// stab tester ^)

int mythread(int param) {
printf("Param here=%d from thread\n",param);
int i; for(i=0;i<10;i++) {
 printf("thr=%d here\n",i);
 //msleep(100);
 //usleep
 }
printf("thread done!\n");
return 0;
}

void dummy(int s) { sleep(s); }

int  count_max_threads() {
int i;
int b = os_ticks();
for(i=0;;i++) {
   printf("Start thread: %d \r",i);
   void *th = thread_create(dummy,(void*)1);
   if (!th) break;
   }
int b2 = os_ticks()-b;
printf("Number of Thread created: %d in %d msec\n",i,b2); // 382 on linux icore7 4gb
b2=os_ticks();
msleep(100);
printf("sleep 100msec is %ld\n",os_ticks()-b2);
return 0;
}


int read_proc() { // read process data in a blob....
int prn(void *a, char *buf,int sz) {
printf("[%*.*s]",sz,sz,buf);
return  0;
}
int code = os_exec("ping www.ya.ru -c 3", prn , 0);
printf("code=%d\n",code);
return 0;
}

int main() {
    return read_proc();
    //return count_max_threads();
    printf("Hello world!\n");
    void *th = thread_create(mythread,0);
    printf("th=%p\n",th);
    char buf[10]; gets(buf);
    thread_destory(th);
    printf("thr destroyed!\n");
    char buf2[10]; gets(buf2);
    return 0;
}
