#include <stdio.h>
#include <stdlib.h>

//#include "vos.c"
//#include "exe.c"
//#include "vs0.c"
//#include "logger.c"
//#include "coders.c"
//#include "httpTest.c"
//#include "httpSrv.c"
//#include "sock.c"
//#include "vss.c"
//#include "std_sock.c"

#include "strutil.h"
#include "sock.h"

int lineReady(char *s) { // zu - to utils???
char *eoh, *r=s;
if (!s) return 0; // no yet
while(*r && *r<=32) r++;
eoh = strstr(r,"\n"); if (!eoh) return 0; // no "end of headers"
return 1+(eoh-s); // return block length
}

int onEchoClientPacket(char *data,int len, Socket *sock) {
int l = lineReady(data);
if (l<=0) return 0; // not yet
data[l]=0;
data=trim(data);
  printf("IN:%s\n",data);
  SocketSendf(sock,"YouTyped:%s\n",data);
  if (lcmp(&data,"exit")) {SocketSendf(sock,"Goodbye"); sock->dieOnSend=1;}
return l;
}

int onEchoClientConnect(Socket *lsock, int handle, int ip) {
Socket *sock;
SocketPool  *srv = (void*)lsock->pool; // Here My SocketServer ???
printf("New connect accept to pool %p\n",srv);
sock = SocketPoolAccept(srv,handle,ip);
if (!sock) { // can be if wrong parameters or no pem.file
   return 0; // killed by SocketPool
   }
SocketSendf(sock,"Hello, echo server\n");
//CLOG(srv,3,"new connect#%d from '%s', accepted\n",sock->N,sock->szip);
printf("new connect#%d from '%s', accepted\n",sock->N,sock->szip);
sock->checkPacket = onEchoClientPacket; // When new packet here...
//if (srv->readLimit.Limit) sock->readPacket = &srv->readLimit; // SetLimiter here (if any?)
return 1; // Everything is done...
}


int SocketEchoServer(int port) {
  Socket *s = SocketNew();
  if (SocketListener(s,port,onEchoClientConnect,lineReady,0)<=0) {
     printf("Fail Listen port %d\n",port); return -1;
  }
printf("Port %d listened OK pool=%p sock=%p handle=%d\n",port,s->pool,s,s->sock);
while(!aborted) {
  //printf("1 pool=%p\n",s->pool);
  TimeUpdate(); // TimeNow & szTimeNow
//printf("2\n");
  int cnt = SocketPoolRun((SocketPool*)s->pool);
  //printf("4\n");
     // cnt+=wsSrvStep(ws);
  //printf("SockPoolRun=%d time:%s\n",cnt,szTimeNow); msleep(1000);
  RunSleep(cnt); // Empty socket circle -)))
  //printf("5\n");
//  if (srv->runTill && TimeNow>=srv->runTill) break; // Done???
  }
//printf("YES\n");
TimeUpdate();
//printf("Server done\n");
return 0;
}

int MicroHttpMain(int npar,char **par) ;
int main(int npar,char **par)
{
    net_init();
    return SocketEchoServer(1234);
    //return MicroHttpMain(npar,par);
}
