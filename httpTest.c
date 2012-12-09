/*
  ћодуль управлени€ клиентским асинхронным соединением:
   1. ќбработка onDisconnect - если вдруг сервер отвалилс€ - нужно уметь к нему законнекчиватьс€?
   2. ќбработка onReplay - при получении ответа от сервера - что делать???


  ¬ажный вопрос - что делать - если потер€но соединение с сервером. ¬ каком состо€нии находились мои милые соединени€???
   ак это вообще можно пон€ть???

  ≈ще одна тема - статистика.  ак можно определить сколько проходит пакетов (в секунду?) - мы можем говорить о том,
  сколько пакетов было в прошлую секунду? - да, она закончилась. —лед - нужно обращатьс€ к каунтеру отсылки???

*/

#include "httpSrv.h"

#ifdef httpTestMain
/* compile with all need sources --- */
#include "exe.c"
#include "sock.c"
#include "httpSrv.c"
#include "vos.c"
#include "vs0.c"
#include "vss.c"
#include "logger.c"
#include "coders.c"
//#pragma comment(lib,"ws2_32")
#endif



/// ---------------  AutoTesting here  ------------------------

int onTestDone(httpCmd *cmd) ; // DoneTestCommand
httpCmd *SendTestPack(httpClient *cli);

extern int aborted;

int onTestDisconnect(httpClient *cli) {
    CLOG(cli,0,"Server disconnected, Abort!!!\n");
    aborted=1;
    return 0;
CLOG(cli,1,"Server disconnected, try reconnect....\n");
if (!httpClientInit(cli,cli->cs,cli->proxy)) {
    CLOG(cli,0,"Fatal - server reconnect failed");
    aborted=1;
    return 0;
    }
CLOG(cli,1,"Server reconnected OK\n");
httpClientClearCommands(cli,0); // RemoveAll Commands from a here ...
SendTestPack(cli) ; // send new test packet???
return 1;
}

httpCmd *SendTestPack(httpClient *cli) {
httpCmd *cmd;
cmd = httpClientGet(cli,0); // GetDefaultPage
cmd->onDone = onTestDone; // Call On End
return cmd;
}

int ReplayCode = 200; // Wait 200 OK

int onTestDone(httpCmd *cmd) { // DoneTestCommand
httpClient *cli = cmd->cli;
Counter *c = cli->sock.readPacket;
if (ReplayCode) { // Check Replay
    int code=0;
    if (cli->res.U.len>0) code=atoi(cli->res.U.data); // GetACode
    if (code!=ReplayCode) {
        CLOG(cli,0,"WrongServerAnswer: %d, expect: %d\n",code,ReplayCode);
        aborted=1;
        }
    }
if(0) printf("CommandDone n=%d readPacket=%d total=%d os_mem=%d!\n",cmd->n,
  c?c->pValue:0,c?c->Total:0,os_mem_used());
SendTestPack(cli);
return 0;
}

#include <signal.h>

int readPackLimit = 100000;

void httpTestHelp() {

    printf("   httpTest - ServerStressTester ('%s','Auth:basic','Proxy: basic')\n",X_REQUEST_ID);
    printf("Usage: httpTest.exe <[http://]URL> [-s<socket_count:1>] [-p<pool_packet:1>] \n "
       " [-d<logLevel:1] [-M<maxPacket:1000000>] [-R<replayCode:200>] [-P<user/pass@proxy>] [-S<sleepMsec:0>]\n"
       " [-r<readPackLimit(persec):100000>]\n"
         );
}



int httpTestMain(int npar,char **par) {
int i,sock=1,pack=1,logLevel=1;
int MaxPacket = 1000000;
char *url,*Proxy="";
net_init();
signal(SIGINT,sig_done);
if (npar<2) {
    httpTestHelp();
    return 1; // Error
    }
char *cmd=par[1];
if (strncmp(cmd,"-p",2)==0) return MicroHttpMain(npar,par);
if (strncmp(cmd,"http://",7)==0) cmd+=7; // RemoveHTTP
url=cmd;
static Counter read={100000}; // ReadLimit
for(i=2;i<npar;i++) { // Decode Optional
    char *cmd=par[i]; int ok=1;
    if (*cmd=='-') {
    cmd++; ok=1;
    if (*cmd=='s') sscanf(cmd+1,"%d",&sock);
    //else if (*cmd=='S') sscanf(cmd+1,"%d",&sleepTime);
    else if (*cmd=='p') sscanf(cmd+1,"%d",&pack);
    else if (*cmd=='d') sscanf(cmd+1,"%d",&logLevel);
    else if (*cmd=='M') sscanf(cmd+1,"%d",&MaxPacket);
    else if (*cmd=='P') Proxy=cmd+1;
    else if (*cmd=='R') sscanf(cmd+1,"%d",&ReplayCode);
    else if (*cmd=='r') sscanf(cmd+1,"%d",&read.Limit);
    else ok=0;
    }
    if (!ok) {
        printf("httpTest - unknownParameter: '%s'\n",cmd);
        httpTestHelp();
        return 2;
        }
    }
//thread_create(httpClientTest1);

httpClient *Cli[sock];
logger *log;
log = logOpen("httpTest.log");
Logf("httpStressTest: {logLevel:%d,Sockets:%d,Commands:%d,MaxPacket:%d,URL:'%s',Proxy='%s'\n",
   logLevel,sock,pack,MaxPacket,url,Proxy);
for(i=0;i<sock;i++) {
     httpClient *c;
     Cli[i]= c = httpClientNew();
     c->log = log;
     c->logLevel = logLevel;
     snprintf(c->name,sizeof(c->name),"[%d]%s",i,url);
     if (!httpClientInit(c,url,Proxy)) return 0;
     c->onDisconnect = onTestDisconnect;
     c->sock.readPacket = &read;

     //c->sock->readPacket.Limit = readPackLimit;
     //printf("Begin?\n");
     int j; for(j=0;j<pack;j++) SendTestPack(c); // SendTestPacks
     //printf("Done?\n");
     }
TimeUpdate(); time_t Started=TimeNow;
printf("StressStarted: %s, mem=%d\n",szTimeNow,os_mem_used());
while(!aborted) {
  int c=0;
  TimeUpdate();
  for(i=0;i<sock;i++) if (httpClientRun(Cli[i])) c++;
  if (read.Total>=MaxPacket) {
      Logf("MaxPacket %d received\n", MaxPacket);
      break;
      }
  //printf("Run=%d\n",c);
  RunSleep(c);
  if (NeedReport()) Reportf("Time:'%s',Packet_Per_Second:%5d,Total:%5d,os_mem:%d",szTimeNow,
     (read.pValue+read.ppValue)/2,read.Total,os_mem_used());
  //if (!c) msleep(1);
  }
TimeUpdate(); int Dur=TimeNow-Started,PPS=0; if (Dur) PPS=read.Total/Dur;
printf("StressStopped:'%s',Dur:%d,PPS:%d,Total:%5d,os_mem:%d",szTimeNow,Dur,PPS,read.Total,os_mem_used());
return 0;
//return httpClientTest1(logLevel,sock,pack,url); // —лушай - а у мен€ в микрохттп нету ли такого - что € останавливаюсь???
}


int onHttpStat(Socket *sock, vssHttp *req, SocketMap *map) { // √енераци€ статистики по серверу
char buf[1024];
httpSrv *srv = sock->pool;
strSetLength(&srv->buf,0); // ClearResulted
sprintf(buf,"{clients:%d,connects:%d,requests:%d,mem:%d,serverTime:'%s',pps:%d}",arrLength(srv->srv.sock)-1,
  srv->srv.connects,
  srv->srv.requests,
  os_mem_used(), szTimeNow,
  (srv->readLimit.pValue+srv->readLimit.ppValue)/2);
SocketPrintHttp(sock,req,"%s",buf); // Flash Results as http
return 1; // OK - generated
}


/// --- microHttp Starting here ...

//time_t TimeNow;
//int aborted;
int port = 80;
int logLevel = 1;
int keepAlive = 1;
int runTill = 0;
char *rootDir = "./";
char *mimes=".htm,.html=text/html;charset=windows-1251&.js=text/javascript;charset=windows-1251";


int MicroHttpMain(int npar,char **par) {
int i,Limit=1000000;
if (npar==1) {
    printf("microHttp.exe -p[port] -r[rootDir] -d[debugLevel] -m[ext=mime[&..]]) -L[limitPPS:1000000]\n");
    return 0;
    }
for(i=1;i<npar;i++) {
    char *cmd = par[i];
    if (*cmd=='-') cmd++;
    switch(*cmd) {
    case 'p': sscanf(cmd+1,"%d",&port); break;
    //case 'S': sscanf(cmd+1,"%d",&sleepTime); break;
    case 'd': sscanf(cmd+1,"%d",&logLevel); break;
    case 'k': sscanf(cmd+1,"%d",&keepAlive); break;
    case 'T': sscanf(cmd+1,"%d",&runTill); break;
    case 'L': sscanf(cmd+1,"%d",&Limit); break;
    case 'r': rootDir=cmd+1; break;
    case 'm': mimes = cmd+1; break;
    }
    }
net_init();
TimeUpdate();
httpSrv *srv = httpSrvCreate(0); // New Instance, no ini
srv->log =  srv->srv.log = logOpen("microHttp.log"); // Create a logger
srv->logLevel = srv->srv.logLevel = logLevel;
srv->keepAlive=keepAlive;
srv->readLimit.Limit = Limit;
IFLOG(srv,0,"...starting microHttp {port:%d,logLevel:%d,rootDir:'%s',keepAlive:%d,Limit:%d},\n   mimes:'%s'\n",
  port,logLevel,rootDir,keepAlive,Limit,
  mimes);
//printf("...Creating a http server\n");
srv->defmime= vssCreate("text/plain;charset=windows-1251",-1);
httpSrvAddMimes(srv,mimes);
//httpMime *m = httpSrvGetMime(srv,vssCreate("1.HHtm",-1));printf("Mime here %*.*s\n",VSS(m->mime));
//httpSrvAddFS(srv,"/c/","c:/",0); // Adding some FS mappings
httpSrvAddFS(srv,"/",rootDir,0); // Adding some FS mappings
httpSrvAddMap(srv, strNew("/.stat",-1), onHttpStat, 0);
if (httpSrvListen(srv,port)<=0) { // Starts listen port
   Logf("-FAIL start listener on port %d\n",port); return 1;
   }
Logf(".. listener is ready, Ctrl+C to abort\n");
if (runTill) srv->runTill = TimeNow + runTill;
httpSrvProcess(srv); // Run All messages till CtrlC...
TimeUpdate();
IFLOG(srv,0,"...stop microHttp, done:{connects:%d,requests:%d,runtime:%d}\n",
     srv->srv.connects,srv->srv.requests,TimeNow - srv->created);
return 0;
}
