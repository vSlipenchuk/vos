#ifndef STRUTIL_H
#define STRUTIL_H

#include "vtypes.h"
// strutil.c

int lcmp(char **str,char *cmp); //@strutil.c
char *get_word(char **str); //@strutil.c gets a word or ""
int get_int(char **str); //@strutil.c


int gsm2utf(char *out,char *ucs2,int len) ;

int hexdump(char *msg,uchar *s,int len);
int hexstr2bin_(char *out,int out_sz,char *in, int len);

char *get_row(char **tbl); //@strutil.c
char *get_col(char **row); //@strutil.c
int get_cols(char **row,char *fmt,...); //@strutil.c



int strnstr(char *str,int sl,char *del,int dl); // Поиск подстроки
char *trim(char *src); // left+right trim
char *ltrim(char *src); // left trim
char *get_till(char **data,char *del,int dl); // split a string...

#endif // CODERS_H
