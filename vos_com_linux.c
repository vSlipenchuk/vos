/*
info about serial:
  cat /proc/tty/driver/serial
*/

#ifdef __linux__


#include "vos.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include <sys/ioctl.h>
#include <sys/types.h>

#include <termios.h>

void static prt_cfg(int ttyDev,int baudrate) {
struct termios oldtio,newtio;

tcgetattr(ttyDev, &oldtio); //backup current settings
newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
newtio.c_cflag &= ~CRTSCTS; //disable hw flow control
newtio.c_iflag &= ~(IXON | IXOFF | IXANY); //disable flow control
newtio.c_iflag |= IGNPAR; //ignore parity
newtio.c_oflag = 0;
newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw mode
newtio.c_cc[VMIN] = 1;
newtio.c_cc[VTIME] = 0;
 //cfsetspeed(&newtio, getBaud(baudrate));
tcflush(ttyDev, TCIFLUSH);
tcsetattr(ttyDev, TCSANOW, &newtio);
//clear RTS
//ioctl(ttyDev, TIOCMBIC, &mcs);
}

void *prt_open (char *name,int speed) {
int h;
char fname[1024];
if (*name!='/') { // add default /dev
  sprintf(fname,"/dev/%s",name); name=fname;
  }
h = open(name,O_RDWR | O_NOCTTY,0);
if (h<0) { return 0;} // error
int mode=B115200; // default mode
switch(speed) {
 case 57600: mode=B57600; break;
 case 19200: mode=B19200; break;
 case 9600:  mode=B9600;  break;
 }
if (speed>0) prt_cfg(h,mode);
return (void*)h;
}

int   prt_peek (void *com,void *buf, int size) {
int h=(int)com; int bytesWaiting=1;
if (h==0) return 0; // invalid
if (ioctl(h, FIONREAD, &bytesWaiting)!=0) return -1;
// if (bytesWaiting) printf("(%d,%d)\n",bytesWaiting,size);
if (bytesWaiting<=0) return bytesWaiting; // not yet
if (bytesWaiting>=size) bytesWaiting=size-1;
memset(buf,0,bytesWaiting);
int r = read(h,buf,bytesWaiting); // EOF=0 reading, like on sockets???
  //printf("\nREAD: %d \n%*.*s\n",r,r,r,buf);
  //hex_dump("com_read ",buf,r);
return r;
}

int   prt_write(void *com,void *buf, int len) {
if (!com) return 0;
  //hex_dump("com_write",buf,len);
return  write((int)com,buf,len);
}

void  prt_close(void *com) {
  if (!com) return ;
  close((int)com);
}

#endif
