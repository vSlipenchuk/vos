#ifndef __linux__

#include <stdio.h>
#include <stdlib.h>
//#include <winsock.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "vos.h"


#include <psapi.h>
#include <windows.h>
int os_mem_used() {
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(),&pmc,sizeof(pmc));
	return pmc.PeakWorkingSetSize;
}

unsigned long os_ticks() { return GetTickCount(); }

char *exe_name; // Aie?ii auou onoaiiaeaii ia eiy exe-i?ia?aiiu
char  exe_compiled[200]; // Aaoa-a?aiy nai?ee. Iiianoeou a enoiaiue oaee


int os_set_compiled(char *date, char *time) { // __DATE__ __TIME__
sprintf(exe_compiled,"%s %s",date,time);
return 0;
}

char **os_env; // for extern settings

char *os_getenv(char *name) {
int i,n;
if (!os_env) return 0;
for(i=0;os_env[i];i++) {
 char *nv = os_env[i];
 for(n=0;nv[n];n++) if (nv[n]=='=') { // name=val
   if (memcmp(name,nv,n)==0) return nv+(n+1);
   break;
   }
 }
return 0;
}

void msleep(int msec) { Sleep(msec);  }
unsigned int  sleep(unsigned int sec) { Sleep(sec*1000); return 0;}

/*
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
*/

void *dl_open(char *name) { return LoadLibrary(name); }
void *dl_sym(void *lib, char *name) { return GetProcAddress(lib,name);}
void dl_close(void *lib) { FreeLibrary(lib);}

// Iioiee

#define VTHREAD_MAX_STACK 1 /* Oieuei iaei ia?aaaaaaiue ?a?ac noae ia?aiao? */


typedef struct {
  HANDLE handle;
  int (*proc)();
  int done;
  int *par;
} thread_create_struct;

typedef struct
{
  int p[VTHREAD_MAX_STACK];
} thread_create_struct_tmp;


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

void *thread_create(void *proc,void *arg) {
  HANDLE tid;
  DWORD  ID;
  //int p[VTHREAD_MAX_STACK];
  thread_create_struct fs;
  fs.proc=proc;
  fs.done=0;
  fs.par=(void*)((int*)(&proc)+1);
  fs.handle=0;
  tid=CreateThread(NULL,0,(void*)(thread_create_do),&fs,0,&ID);
  if(!tid){  /*errno=GetLastError(); */ return 0; }
  fs.handle=tid;
  while(!fs.done) Sleep(1);
  return (void*)tid;
}

int thread_suspend(void* thread){ return SuspendThread((HANDLE)thread); }
int thread_resume(void* thread){  return ResumeThread((HANDLE)thread);  }
int thread_destroy(void *thread,int code) { return TerminateThread((HANDLE)thread,code); }
void *thread_self() { return (void*) GetCurrentThreadId(); }

/*
int proc_create(os_proc *p, char *cmd ) { // Caione oaeea, iieo?aiea i?ioanna e auoiaiiai iioiea
  int i;  SECURITY_ATTRIBUTES sa; DWORD mode,mchar,mwait;
  STARTUPINFO si;  PROCESS_INFORMATION pi; int *ppp;
  HANDLE wh; // Eoaa ieoao i?ioann
  sa.nLength=sizeof(sa);
  sa.lpSecurityDescriptor=NULL;
  sa.bInheritHandle=TRUE;
  memset(p,0,sizeof(*p));
  CreatePipe(&p->hOut,&wh,&sa,1); // Nicaaai i?ia?aiiio? o?oao
  si.cb=sizeof(si);           // ?aci_? no?oeoo?u
  si.lpReserved=NULL;         // must be NULL
  si.lpDesktop=NULL;          // "Desktop[\WindowStation]" -an<_ao?ony
  si.lpTitle=NULL;            // NULL a<y GUI e a_ceR-nR<u-uo
  si.dwX=0; si.dwY=0; si.dwXSize=0; si.dwYSize=0; // ignored
  si.dwXCountChars=80; si.dwYCountChars=25;       // ignored
  si.dwFillAttribute=0;                           // ignored
  // -an<_aRaaou stdin,stdout,stderr;  eniR<ucRaaou ?_?ei iReaca Re-a
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
  p->hProcess = pi.hProcess; // Eiie?oai i?ioann
  CloseHandle(wh);
 // mode=PIPE_READMODE_BYTE|PIPE_NOWAIT; mchar=1; mwait=1;
 // if (!SetNamedPipeHandleState(p->hOut,&mode,&mchar,&mwait)) {
   //   printf("SetPipeFailed!\n");getch();
     // }
return 1;
}

int proc_read(os_proc *p, char *buf, int len) { // ?eoaai ec i?ioanna (out)
long nn;
if (!p->hOut) return -2; // O?a cae?uo
if (!PeekNamedPipe(p->hOut, 0,0,0,  &nn, 0)) {
    proc_destroy(p,1); return -1; // error !!!
    }
if (!nn) return 0; // no yet
if (len>nn) len = nn; // no more than ready bytes
if (!ReadFile(p->hOut,buf,len,&nn,0)) { proc_destroy(p,1); return -2; }
return nn; // Neieuei oaaeinu i?i?eoaou
}

int proc_destroy(os_proc *p, int abort) {
int hres;
if (p->hOut) {
    CloseHandle(p->hOut); p->hOut=0;}
if (p->hProcess) {
     if(!GetExitCodeProcess((void*)p->hProcess,(LPDWORD)(&hres))) {
         p->code=-3; // ???
         }
     p->code = hres; // Noaaei eia aica?aoa
     if (hres==STILL_ACTIVE && abort) {
         printf("Terminate a process!\n");
         TerminateProcess(p->hProcess,0);
         }
     CloseHandle(p->hProcess); p->hProcess=0;
     }
return 1;
} // Oeia caaa?oaao i?ioann
*/

// -- mutex

void *mutex_create() {
  CRITICAL_SECTION *cs;
  cs=malloc(sizeof(CRITICAL_SECTION));
  if(!cs) return 0;
  memset(cs,0,sizeof(CRITICAL_SECTION));
  InitializeCriticalSection(cs);
  return cs;
}


void mutex_lock(void *mutex){
  if(!mutex) return;
  EnterCriticalSection(mutex);
  return ;
}


void mutex_unlock(void *mutex) {
  if(!mutex) return ;
  LeaveCriticalSection((void*)mutex);
  return ;
}


void mutex_destroy(void * mutex)
{
  if(!mutex) return ;
  DeleteCriticalSection(mutex);
  free(mutex);
  return ;
}



#endif
