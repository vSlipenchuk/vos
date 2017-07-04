#ifndef VTYPES_H_INCLUDED
#define VTYPES_H_INCLUDED

// common types
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __linux__
#include <windows.h>
#define MSWIN
#endif

// common header for add-on-types

typedef unsigned char uchar;
typedef unsigned char byte;


typedef struct { // common blob
   int size,len;
   uchar *data;
   int handle;
   } t_blob;

typedef struct { // string-part structure
   int len; uchar *data;
   } vss;

// object-data definitions (vs0) ?

typedef struct {
    int Limit;
    int Total;
    int Modified;
    int Value,pValue,ppValue;
    } Counter;


#define cast(type,obj) ((type)(obj))

// ToDo - some arch has backward stacks!, now think abot intel
#define ptrMove(ptr, shift) cast( void*, ( cast(char*, ptr)+(shift)))
#define ptrShift(base,ptr) (cast(char*,ptr)-cast(char*,base))


// some externals

extern int aborted;
extern time_t TimeNow;
extern char szTimeNow[2*18],szTime2Now[2*18];
void TimeUpdate();
void Reportf(uchar *fmt,...);
void sig_done(int); // default aborted=1 handler
void RunSleep(int cnt);

// some simple declares

#define BUF_FMT(buf,fmt) {va_list va; va_start(va,fmt); _vsnprintf((char*)buf,sizeof(buf)-1,(char*)fmt,va); buf[sizeof(buf)-1]=0; va_end(va);}

#ifndef strNcpy
#define strNcpy(A,B) { strncpy(A,B,sizeof(A)-1); A[sizeof(A)-1]=0;}
#endif


#endif // VTYPES_H_INCLUDED
