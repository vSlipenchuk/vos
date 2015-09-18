#include "vos.h"
#include "vtypes.h"
#include <sys/stat.h>



#ifndef __linux__
int net_init() {
WSADATA wsadata;
return WSAStartup(MAKEWORD(2,2),&wsadata)==0;
}
#else
int net_init() { // ZU - init number of sockets allowed for app?
return 0;
}
#endif


int sock_listen(int port, int jobs) { // Create a socket for listen
int sock;
struct sockaddr s;
sock=socket(AF_INET,SOCK_STREAM,0);
if (sock<0) return -1;
s.sa_family=AF_INET;
*(short*)(s.sa_data)=htons((short)port);
*(int*)(&s.sa_data[2])=0;
#ifndef MSWIN
int on = 1; setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
#endif
if (bind(sock,&s,sizeof(s))) {   closesocket(sock);  return -2;  }
if (listen(sock,jobs))       {   closesocket(sock);  return -3;  }
return sock;
}

int sock_connected(int sock) {
int l,s[2]={1,sock};//,s2[2]={1,sock}; // fdset
  struct timeval t={0,0};
if ( select(sock,(void*)s,0,0,&t)!=1) return 0;
//return 1;
l = recv(sock,(void*)s,1,MSG_PEEK);
if (l==0) {
  //printf("EOF on socket %d\n",sock);
  return -1; // End Of File?
  }
return l>0;
}

int sock_async(int sock) {
#ifndef __linux__
int val=1;
return ioctlsocket(sock, FIONBIO , (void*)&val)==0;
#else
return fcntl(sock, F_SETFL, O_NONBLOCK); // =х сыюъшЁє¦•шщ ёюъхЄ...
#endif
}


int _sock_connect(int sock, unsigned char *cs,int cl, int port) {
uchar host[80],*p;
struct sockaddr s;
struct hostent *h;
int addr;
if (cl<0) cl=strlen(cs);
if (cl>79) cl=79;
memcpy(host,cs,cl); host[cl]=0;
p=strchr(host,':');
if (p) {*p=0; sscanf(p+1,"%d",&port);}
memset(&s,0,sizeof(s));
s.sa_family=AF_INET;
*(short*)(s.sa_data)=htons((short)port);
addr=inet_addr(host);
//printf("ADDR=%d for host=%s\n",addr,host);
if (addr != INADDR_NONE)  {
 *(int*)(&s.sa_data[2])=addr;
 if (connect(sock,&s,sizeof(s))) {
      printf("fail connect to addr %s port %d code %d\n",host,port,addr);
      return 0;
      }
 return 1;
 }
h=gethostbyname(host);
if (!h) {
    printf("get hostbyname failed");
    return 0;
    }
//h->
memcpy(&s.sa_data[2],&h->h_addr_list,4);
*(int*)(&s.sa_data[2])=*(int*)(h->h_addr_list[0]);
if (connect(sock,&s,sizeof(s))) {
     return 0;
     printf("fail connect to namehost %s port %d\n",addr,port);
     }
return 1;
}

int sock_connect(char *host,int defPort) {
int sock;
sock=socket(AF_INET,SOCK_STREAM,0);
if (sock<0) return -1;
if (!_sock_connect(sock, host, strlen(host), defPort)) {
    sock_close(sock); return 0;
    }
return sock;
}


int sock_acceptable(int sock) {
return sock_readable(sock);
}

int sock_readable(int sock) {
fd_set fs;
FD_ZERO(&fs);
FD_SET(sock,&fs);
struct timeval t={0,0};
int res = select(sock+1,&fs,0,0,&t);
//printf("Readable=%d for sock=%d\n",res,sock);

return res;
}


int sock_writable(int sock) {
fd_set fs;
FD_ZERO(&fs);
FD_SET(sock,&fs);
struct timeval t={0,0};
return select(sock+1,0,&fs,0,&t);
}

int sock_accept(int lsock, int *ip) {
struct sockaddr sa; int slen;
int sock;
slen = sizeof(sa);
sock = accept(lsock, &sa,&slen);
if (sock>0 && ip) { *ip = *(int*)(&sa.sa_data[2]);  } // Copy IP address
return sock;
}

int sock_read(int sock, char *buf, int len) { // Async read from a socket
int l;
l = sock_readable(sock);
if (l<=0) return l; // done
return recv(sock,buf,len,0);
}

int sock_write(int sock, char *buf, int len) { // Async read from a socket
int l;
l = sock_writable(sock);
if (l<=0) return l; // done
return send(sock,buf,len,0);
}

int sock_close(int sock) {
//printf("sock %d closed\n",sock);
shutdown(sock,3);
closesocket(sock);
return 0;
}

#ifdef MSWIN

int tick_count() {
return GetTickCount();
}

int file_modified(char *name) { // Дата модификации файла(по имени)
struct stat st;
if (stat(name,&st)!=0) return 0; // Failed get a stat
return st.st_mtime;
}

int os_sysdate(char *buf) {
time_t t; struct tm Now;
time(&t);
if (buf) {
Now = *localtime(&t);
sprintf(buf,"%d.%02d.%04d %02d:%02d:%02d",Now.tm_mday,Now.tm_mon+1,Now.tm_year+1900,
 Now.tm_hour,Now.tm_min,Now.tm_sec);
 }
return (int)t;
}

int os_sysdate2(char *buf) {
time_t t; struct tm Now;
time(&t);
if (buf) {
Now = *localtime(&t);
sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",Now.tm_year+1900,Now.tm_mon+1,Now.tm_mday,
 Now.tm_hour,Now.tm_min,Now.tm_sec);
 }
return (int)t;
}

int os_systime(char *buf) {
time_t t; struct tm Now;
time(&t);
if (buf) {
Now = *localtime(&t);
sprintf(buf,"%02d:%02d:%02d",Now.tm_hour,Now.tm_min,Now.tm_sec);
 }
return (int)t;
}



int os_time() { time_t t; time(&t); return (int)t;}

/*
void *dl_open(char *name,int mode) { return LoadLibrary(name); }
void *dl_sym(void *lib, char *name) { return GetProcAddress(lib,name);}
void dl_close(void *lib) { FreeLibrary(lib);}
*/
// Потоки

#define VTHREAD_MAX_STACK 1 /* Только один передаваемый через стек параметр */




/*
typedef struct {
  int p[VTHREAD_MAX_STACK];
} thread_create_struct_tmp;

DWORD WINAPI thread_create_do(void *fs) {
  thread_create_struct_tmp p;
  HANDLE handle;
  memcpy(&p,((thread_create_struct*)fs)->par,sizeof(p));
  while(!((thread_create_struct*)fs)->handle) Sleep(1);
  handle=((thread_create_struct*)fs)->handle;
  ((thread_create_struct*)fs)->done=1;
  ((thread_create_struct*)fs)->proc((void*)p.p[0]); //,p.p[1],p.p[2]); // ZU?
  CloseHandle(handle);
  return 0;
}




void process_destroy(int code)
{
  ExitProcess(code);
}


DWORD WINAPI thread_create_do(thread_create_struct *fs) {
  thread_create_struct_tmp p;
  HANDLE handle;
  memcpy(&p,fs->par,sizeof(p));
  while(!fs->handle) Sleep(1);
  handle=fs->handle;
  fs->done=1;
  fs->proc(p);
  CloseHandle(handle);
  return 0;
}

void *thread_create (void *_proc, void *arg) {
//void *thread_create(int (*proc)(void*,...),...) {
  int (*proc)() = _proc ;
  HANDLE tid;
  DWORD  ID;
  //int p[VTHREAD_MAX_STACK];
  void *pp = &proc;
  thread_create_struct fs;
  fs.proc=proc;
  fs.done=0;
  fs.par=(void*)((int*)(pp)+1);
  fs.handle=0;
  tid=CreateThread(NULL,0,(void*)(thread_create_do),&fs,0,&ID);
  if(!tid){  return 0; }
  fs.handle=tid;
  while(!fs.done) Sleep(1);
  return (void*)tid;
}

int thread_suspend(void* thread){ return SuspendThread((HANDLE)thread); }
int thread_resume(void* thread){  return ResumeThread((HANDLE)thread);  }
int thread_destroy(void *thread,int code) { return TerminateThread((HANDLE)thread,code); }
void *thread_self() { return (void*) GetCurrentThreadId(); }
*/

typedef struct {
  HANDLE hIn, hOut;
  HANDLE hProcess;
  int code;
  } os_proc;

int proc_create(os_proc *p, char *cmd ) { // Запуск файла, получение процесса и выходного потока
  SECURITY_ATTRIBUTES sa;
  STARTUPINFO si;  PROCESS_INFORMATION pi; //int *ppp;
  HANDLE wh; // Куда пишет процесс
  sa.nLength=sizeof(sa);
  sa.lpSecurityDescriptor=NULL;
  sa.bInheritHandle=TRUE;
  memset(p,0,sizeof(*p));
  CreatePipe(&p->hOut,&wh,&sa,1); // Создаем программную трубу
  si.cb=sizeof(si);           // разм_р структуры
  si.lpReserved=NULL;         // must be NULL
  si.lpDesktop=NULL;          // "Desktop[\WindowStation]" -ас<_дуются
  si.lpTitle=NULL;            // NULL д<я GUI и б_зкR-сR<ь-ых
  si.dwX=0; si.dwY=0; si.dwXSize=0; si.dwYSize=0; // ignored
  si.dwXCountChars=80; si.dwYCountChars=25;       // ignored
  si.dwFillAttribute=0;                           // ignored
  // -ас<_дRвать stdin,stdout,stderr;  испR<ьзRвать р_жим пRказа Rк-а
  si.dwFlags=STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.wShowWindow=SW_SHOWMINNOACTIVE; // SW_HIDE
  si.cbReserved2=0;           // must be 0
  si.lpReserved2=NULL;        // must be NULL
  si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput=wh; //GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError=wh; //GetStdHandle(STD_ERROR_HANDLE);
  if(!CreateProcess(NULL,cmd,NULL,NULL,TRUE,0,NULL,0,&si,&pi)) {
         CloseHandle(p->hOut);
         CloseHandle(wh);
         return 0;
         }
  p->hProcess = pi.hProcess; // Копируем процесс
  CloseHandle(wh);
 // mode=PIPE_READMODE_BYTE|PIPE_NOWAIT; mchar=1; mwait=1;
 // if (!SetNamedPipeHandleState(p->hOut,&mode,&mchar,&mwait)) {
   //   printf("SetPipeFailed!\n");getch();
     // }
return 1;
}

int proc_read(os_proc *p, char *buf, int len) { // Читаем из процесса (out)
long nn;
if (!p->hOut) return -2; // Уже закрыт
if (!PeekNamedPipe(p->hOut, 0,0,0,  &nn, 0)) {
    proc_destroy(p,1); return -1; // error !!!
    }
if (!nn) return 0; // no yet
if (len>nn) len = nn; // no more than ready bytes
if (!ReadFile(p->hOut,buf,len,&nn,0)) { proc_destroy(p,1); return -2; }
return nn; // Сколько удалось прочитать
}

int proc_destroy(os_proc *p, int abort) {
int hres;
if (p->hOut) {
    CloseHandle(p->hOut); p->hOut=0;}
if (p->hProcess) {
     if(!GetExitCodeProcess((void*)p->hProcess,(LPDWORD)(&hres))) {
         p->code=-3; // ???
         }
     p->code = hres; // Ставим код возврата
     if (hres==STILL_ACTIVE && abort) {
         printf("Terminate a process!\n");
         TerminateProcess(p->hProcess,0);
         }
     CloseHandle(p->hProcess); p->hProcess=0;
     }
return 1;
} // Типа завершает процесс

//#include "mutex-1.1.c"

#endif // MSWIN

// -- udp from 1.0

int udp_sock(int port,uchar *ip) { // Create UDP sock & starts listen (if need)
int sock;
struct sockaddr s;
sock=socket(AF_INET,SOCK_DGRAM,0);
if (port>0) { // bind to port ...
	net_sa(&s,ip,port);
	if (bind(sock,&s,sizeof(s))) {   closesocket(sock);  return -2;  }
	//if (listen(sock,8))   {   closesocket(sock);  return -3;  }
	}
return sock;
}

uchar *sa2str(struct sockaddr *sa,uchar *str) {
static uchar szstr[80];
if (!str) str = szstr;
// ok - need to transform???
uchar *ip  = (void*)(&sa->sa_data[2]);
int port = ntohs(*(short*)(sa->sa_data));
sprintf(str,"%d.%d.%d.%d:%d",ip[0],ip[1],ip[2],ip[3],port);
return str;
}

int get_local_ip(char * ip) {
   struct hostent *phost;
   struct in_addr addr;
   char hostname[100];
   if (gethostname(hostname, 100)!=0) return -1;
   phost = gethostbyname(hostname);
   if(!phost) return -2;
   memcpy(&addr, phost->h_addr_list[0], sizeof(struct in_addr));
   uchar *szaddr=inet_ntoa(addr);
   strcpy(ip, szaddr);
   return 1;
}

int net_sa_(void *sa,int addr, int port) { // Compose socket addr ..
struct sockaddr *s=sa;
memset(s,0,sizeof(struct sockaddr));
s->sa_family = AF_INET;
*(short*)(s->sa_data)=htons((short)port);
*(int*)(&s->sa_data[2])=addr;
//printf("SA=%d,port=%d\n",addr,port);
return addr;
}

int net_sa(void *sa,char *host, int port) { // Compose socket addr ..
char szhost[200];
int    addr = 0;
struct sockaddr *s=sa;
if (host) {
 strncpy(szhost,host,199); szhost[199]=0;
 host = strchr(szhost,':');
 if (host) { *host=0; sscanf(host+1,"%d",&port); }
 addr = inet_addr(szhost);
 if (addr<0) { // Try host entry ... -1 - invalid address...
	struct hostent *h;
	h=gethostbyname(szhost);
        if (h) addr = *(int*)(h->h_addr_list[0]); // Host found !!
	}
 //printf("addr=%d for %s\n",addr,szhost);
 }
memset(s,0,sizeof(struct sockaddr));
s->sa_family = AF_INET;
*(short*)(s->sa_data)=htons((short)port);
*(int*)(&s->sa_data[2])=addr;
//printf("SA=%d,port=%d\n",addr,port);
return port;
}

