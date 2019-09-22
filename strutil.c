#include "coders.h"
#include <stdarg.h>

char *delims=" \t\r\n";

uchar *ltrim(uchar *src) {
while(*src&&strchr(delims,*src)) src++;
return src;
}

uchar *rtrim(uchar *src) {
int i;
for(i=0;src[i];i++);
while(i>0 && strchr(delims,src[i-1])) i--;
src[i]=0;
return src;
}

uchar *trim(uchar *src) { return rtrim(ltrim(src)); }

int lcmp(char **str,char *cmp) {
int l = strlen(cmp);
if (memcmp(*str,cmp,l)==0) {
   cmp=*str+l; while(*cmp&& strchr(delims,*cmp)) { cmp++; l++;} // ltrim
   *str=cmp;
   return l; // OK
   }
return 0;
}

uchar *get_row(uchar **tbl) {
uchar *str = *tbl,*ret=str; int i;
for(i=0;str[i]&&str[i]!='\n';i++); // till the end
if (str[i]) { str[i]=0; if (i>0 && str[i-1]=='\r') str[i-1]=0; i++; }
str+=i;
*tbl=str; // to the end
return ret;
}

uchar *get_col(uchar **row) {
uchar *str = *row,*ret=str; int i;
for(i=0;str[i]&&str[i]!='\t';i++); // till the end
if (str[i]) { str[i]=0; i++;}
*row=str+i; // to the end
c_decode(ret,ret,-1);
return ret; // ok
}

int get_cols(uchar **row,uchar *fmt,...) {
int cnt=0; va_list va; void *p;
va_start(va,fmt);
while(*fmt) {
    uchar *col;
    p = va_arg(va,void*);
    //if (*row[0]==0) return cnt; // eof ???
    col = get_col(row);
    //printf("EX:%c,COL:%s\n",*fmt,col);
    switch(*fmt) {
    case 'i':
      if (sscanf(col,"%d",(int*)p)!=1) return -2;
      break;
    case 's':
      c_decode(col,col,-1); // decode C constructs
      *(uchar**)p=col; // remember it here
      break;
    default: return -1; // unknown format
    }
    fmt++; cnt++;
    }
va_end(va);
return cnt;
}


char *get_word(char **str) { // gets a word or ""
char *r = *str,*ret;
//printf("R=<%s>\n",r);
while(*r && strchr(delims,*r) ) r++; // ltrim
ret = r;
while(*r && !strchr(delims,*r)) r++; // collect
if (*r)  { *r=0; r++;}
while(*r && strchr(delims,*r)) r++; // ltrim again
*str = r;
//printf("R=<%s>\n",r);
return ret;
}

int get_int(char **str) {
int ret=0;
sscanf(get_word(str),"%d",&ret);
return ret;
}

int strnstr(uchar *str,int sl,uchar *del,int dl) { // Поиск подстроки
if (sl<0) sl = strlen(str); if (dl<0) dl=strlen(del);
int i;
for(i=0;i<sl-dl+1;i++) if (memcmp(str+i,del,dl)==0) return i;
return -1;
}

uchar *get_till(uchar **data,uchar *del,int dl) {
uchar *str = *data; int sl,ipos;
if (dl<0) dl = strlen(del); sl = strlen(str);
ipos = strnstr(str,sl,del,dl);
//printf("get_till ipos=%d in %s the %s\n",ipos,str,del);
if (ipos<0) {  *data=str+sl; return str; } // return all
str[ipos]=0; *data=str+ipos+dl; // remove delimiter
return str;
}



int get_mime_len(uchar **data) { // Возвращает строку миме-параметра
uchar *str = *data; int sl,len;
sl = strlen(str); len=0; // Будем коллектить длину -)))
while(sl>0) {
  int ipos = strnstr(str,sl,"\r\n",-1);
  if (ipos<0) { len+= sl; break;} // Больше строк нет - возвращаем все...
  ipos+=2; str+=ipos; sl-=ipos; len+=ipos;
  if (sl==0 || *str>32) break; // Следующая строка - начало нового параметра...
  }
return len;
}

