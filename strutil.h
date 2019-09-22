#ifndef STRUTIL_H
#define STRUTIL_H

#include "vtypes.h"
// strutil.c

int lcmp(char **str,char *cmp); //@strutil.c
char *get_word(char **str); //@strutil.c gets a word or ""
int get_int(char **str); //@strutil.c


int gsm2utf(char *out,char *ucs2,int len) ;

int hexdump(char *msg,uchar *s,int len);
int hexstr2bin_(unsigned char *out,int out_sz,unsigned char *in, int len);

uchar *get_row(uchar **tbl); //@strutil.c
uchar *get_col(uchar **row); //@strutil.c
int get_cols(uchar **row,uchar *fmt,...); //@strutil.c



int strnstr(uchar *str,int sl,uchar *del,int dl); // Поиск подстроки
uchar *trim(uchar *src); // left+right trim
uchar *ltrim(uchar *src); // left trim
uchar *get_till(uchar **data,uchar *del,int dl); // split a string...

#endif // CODERS_H
