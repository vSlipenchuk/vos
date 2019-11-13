#ifndef VOS_H_INCLUDED
#define VOS_H_INCLUDED

#ifdef  __linux__
#include "vtypes.h" // add my types
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/limits.h>

#define MAX_PATH PATH_MAX
#define _vsnprintf vsnprintf
#define closesocket close
#define stricmp(str1,str2) strcasecmp(str1,str2)
#define strnicmp(str1,str2,len) strncasecmp(str1,str2,len)
#else
#define MSG_NOSIGNAL 0
#endif

#ifdef __WIN32__ // Win32 or WinCE here
#include <windows.h>


#else // LINUX or UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define O_BINARY 0
int filelength(int file);
#endif


// threads
void *thread_create (void *proc, void *arg); // creates a new thread (and returns it handler), or return NULL (on ERROR)
//void  thread_suspend(void *thread); // win-only?
//void  thread_resume (void *thread); // win-only?
void  thread_destory(void *thread);

// mutex [ reentrant mutex ]
void *mutex_create ();
void  mutex_lock   (void *mutex);
void  mutex_unlock (void *mutex);
void  mutex_destroy(void * mutex);

// time & timers
void msleep(int msec); // msec { usleep(1000*msec); }
unsigned long os_ticks(); //GetTickCount()


// exec programs - to get out in a buffer
int os_exec(char *cmd, int (*on_out)(), void *arg); // on_out ret 0 = ok, continue

// anyc com-ports wrapping (vos_com_[OS].c
void *prt_open (char *name,int speed); // 0=error
int   prt_peek (void *com,void *buf, int bytes); // (AsyncRead) 0=not-ready, <0 error
int   prt_write(void *com,void *buf, int bytes); // <0 error
void  prt_close(void *com);

// term extenstions (win like)
int kbhit();

int os_mem_used();


// sockets

int net_init();
int sock_async(int sock);
int sock_listen(int port, int jobs);
int sock_accept(int lsock, int *ip);
int sock_readable(int sock);
int sock_writable(int sock);
int sock_close(int sock);
int _sock_connect(int sock, char *cs,int cl, int port);


/* assigns UDP socket, if port>0 starts listen, if ip!=NULL binds to signle interface */
int udp_sock(int port,char *ip) ;
int net_sa(void *sa,char *host, int port); // compose socket addr from host:port

// timing



extern int sleepTime; // defined in exe.c


// SSL - if need

#ifdef VOS_SSL
#include <openssl/ssl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


typedef struct {
    SSL_CTX *pCtx;
    BIO *pbioRead;
    BIO *pbioWrite;
    SSL *pSSL;
    int isServer;
    } SSLStateMachine;


void ssl_sock_init(); // init all ssl libraries

SSLStateMachine *SSLStateMachine_new(const char *szCertificateFile,     const char *szKeyFile,int isServer);
void  SSLStateMachine_done(SSLStateMachine *pMachine);

void SSLStateMachine_read_inject(SSLStateMachine *pMachine, const unsigned char *aucBuf,int nBuf);
int SSLStateMachine_read_extract(SSLStateMachine *pMachine,	 unsigned char *aucBuf,int nBuf);
int SSLStateMachine_write_can_extract(SSLStateMachine *pMachine);
int SSLStateMachine_write_extract(SSLStateMachine *pMachine, unsigned char *aucBuf,int nBuf);
void SSLStateMachine_write_inject(SSLStateMachine *pMachine, const unsigned char *aucBuf,int nBuf);
// ZU ssl mashine done




#endif // VOS_SSL



#endif // VOS_H_INCLUDED
