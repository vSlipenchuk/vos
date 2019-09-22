/*
info about serial:
  cat /proc/tty/driver/serial
*/

//#ifdef WIN


#include "vos.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
//#include <sys/ioctl.h>
#include <sys/types.h>
#include <windows.h>

//#include <termios.h>

/*
void  prt_cfg(int ttyDev,int baudrate) {
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
*/


void *prt_open (char *name,int speed) {
LPCSTR portname = name;
DWORD  accessdirection =GENERIC_READ | GENERIC_WRITE;
HANDLE hSerial = CreateFile(portname,
    accessdirection,
    0,
    0,
    OPEN_EXISTING,
    0,
    0);
if (hSerial == INVALID_HANDLE_VALUE) {
    printf("FAIL OPEN COM_PORN %s\n",name);
        return 0;
    //call GetLastError(); to gain more information
     }
DCB dcbSerialParams = {0};
dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
if (!GetCommState(hSerial, &dcbSerialParams)) {
     //could not get the state of the comport
  }
dcbSerialParams.BaudRate=speed;
dcbSerialParams.ByteSize=8;
dcbSerialParams.StopBits=ONESTOPBIT;
dcbSerialParams.Parity=NOPARITY;
if(!SetCommState(hSerial, &dcbSerialParams)){
     printf("FailSet speed %d\n",speed); //analyse error
     return 0;
}
 COMMTIMEOUTS timeouts={0};
timeouts.ReadIntervalTimeout=50;
timeouts.ReadTotalTimeoutConstant=50;
timeouts.ReadTotalTimeoutMultiplier=10;
timeouts.WriteTotalTimeoutConstant=50;
timeouts.WriteTotalTimeoutMultiplier=10;
if(!SetCommTimeouts(hSerial, &timeouts)){
   printf("Fail set timeouts on com port\n"); //handle error
   return 0;
}
return hSerial;
}


int   prt_peek_len(void *com,void *buf, int size)
   // DWORD readFromSerialPort(HANDLE hSerial, uint8_t * buffer, int buffersize)
{
    DWORD dwBytesRead = 0;
    if(!ReadFile(com, buf, size, &dwBytesRead, NULL)){
       printf("READ COM ERROR\n");
       return -1;
    }
    return dwBytesRead;
}


int   prt_write(void *com,void *buf, int len) {
if (!com) return 0;
DWORD dwBytesW = 0;
	if(!WriteFile(com, buf, len, &dwBytesW, NULL)){
		//printLastError();
		return -1;
	}
return dwBytesW;
}

void  prt_close(void *com) {
  if (!com) return ;
  CloseHandle(com);

}

//#endif
