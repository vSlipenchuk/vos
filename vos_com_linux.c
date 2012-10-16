/*
info about serial:
  cat /proc/tty/driver/serial
*/
#include "vos.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/ioctl.h"
#include "sys/types.h"

void *prt_open (char *name) {
int h;
char fname[1024];
if (*name!='/') { // add default /dev
  sprintf(fname,"/dev/%s",name); name=fname;
  }
h = open(name,O_RDWR | O_NOCTTY,0);
if (h<0) { return 0;} // error
return (void*)h;
}

int   prt_peek (void *com,void *buf, int size) {
int h=(int)com; int bytesWaiting=1;
if (h==0) return 0; // invalid
if (ioctl(h, FIONREAD, &bytesWaiting)!=0) return -1;
if (bytesWaiting<=0) return bytesWaiting; // not yet
if (bytesWaiting>size) bytesWaiting=size;
return read(h,buf,bytesWaiting); // EOF=0 reading, like on sockets???
}

int   prt_write(void *com,void *buf, int len) {
if (!com) return 0;
return  write((int)com,buf,len);
}

void  prt_close(void *com) {
  if (!com) return ;
  close((int)com);
}

