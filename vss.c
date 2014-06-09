#include "vss.h"
#include "vos.h"

// Функции работы с указателями на куски текста

vss vssCreate(char *data, int len) { vss out; if (!data) data=""; out.data=data; if (len<0) len = strlen(data); out.len=len; return out; }

vss vssGetLen(vss *src, int len) { // Извлекает len bytes из массива
vss ret = *src;
if (len<0) len = src->len; // Забрать все
if (len>src->len) len = src->len; // Если слишком много - то забрать все
ret.len = len; src->len-=len; src->data+=len;
return ret;
}

vss vssGetTillStr(vss *src, char *div, int dl, int remDiv) {
vss r; int i;
if (dl<0) dl = strlen(div);
for(i=0;i<=src->len-dl;i++) if (memcmp(src->data+i,div,dl)==0) { // Found !!!
    r.data = src->data;  r.len = i;
    if (remDiv) i+=dl;
    src->data+=i;  src->len -=i;
    return r;
    }
r = *src; src->data = 0; src->len = 0; // Not found, return ALL
return r;
}

vss vssGetRow2(vss *src) { // With delimiters
int i; vss out;
for(i=0;i<src->len;i++) if (src->data[i]=='\n') break;
out = *src;
if (i==src->len) { // last line
    src->data+=i; src->len=0; return out;
    }
i++; out.len = i; src->data+=i; src->len-=i;
return out;
}

vss vssGetRow(vss *src) {
vss out;
out = vssGetRow2(src);
if (out.len>0 && out.data[out.len-1]=='\n') out.len--;
if (out.len>0 && out.data[out.len-1]=='\r') out.len--;
return out;
}

vss vssGetCol(vss *src) {
return vssGetTillStr(src,"\t",1,1);
}

int vssTrim(vss *src) {
int r = 0;
while(src->len && *src->data <=32) { src->len--; src->data++; r++;} // ltrim
while(src->len && src->data[src->len-1]<=32) { src->len--; r++;} // rtrim
return r;
}

vss vssGetPropValue(vss *PROP2, char *name, int nl) {
vss R,n;
if (nl<0) nl = strlen(name);
while(PROP2->len>0) {
    R = vssGetRow(PROP2);
    n = vssGetTillStr(&R,"\t",1,1);
    if (n.len == nl && memcmp(name,n.data,nl)==0) return R;
    }
return *PROP2; // Not found
}

int vssHasPropValue(vss *PR, char *name, int nl,vss *Res) {
vss R,PROP2,n;
if (nl<0) nl = strlen(name);
PROP2=*PR;
while(PROP2.len>0) {
    R = vssGetRow(&PROP2);
    n = vssGetTillStr(&R,"\t",1,1);
    if (n.len == nl && memcmp(name,n.data,nl)==0) { *Res=R; return 1;}
    }
return 0;
}


vss vssGetSQLParam(vss *SQL) { // Перебирает параметры и куски текста
uchar *sql = SQL->data;
int i,j,len = SQL->len;
vss out = *SQL;
if ( len >0 && sql[0]==':') { // Извлекаем параметр
    for(i=1; i<len && sql[i]>32 && !strchr(":,+-/*><=!()';{}|",sql[i]);i++);  // Длина до делимитера
    out.len = i; SQL->data+=i; SQL->len-=i; return out;
    }
for(i=0;i<len&&sql[i]!=':';i++) {
    if (sql[i]=='\'' || sql[i]=='"') { // Коллектим строку
        for(j=1;j<len;j++) if (sql[i+j]==sql[i]) break;
        if (j<len) j++; i+=j-1;
        }
    }
//if (i>len) i=len; // На всякий случай ...
out.len = i; SQL->data+=i; SQL->len-=i; return out;
}

vss vssMimeSection(vss *src, vss *del) {
vss row,out;
out.data=src->data; out.len = 0; // Copy all as heads
while(src->len>0) {
    row = vssGetRow2(src); // Row with delimiters
    if (row.len>=del->len && memcmp(row.data,del->data,del->len)==0) { // Stop section
        return out;
        }
    out.len+=row.len;
    }
return out;
}


vss  getMimeLine(vss *src) { //  Заканчивается строкой с непустым первым символом
vss out=*src; int i;
for(i=0;i<src->len;i++) if (i>0 && src->data[i-1]=='\n' && src->data[i]>32) break; // Готово
out.len=i; src->data+=i; src->len-=i;
vssTrim(&out);
return out;
}

int vssGetMimeHeader(vss  *src, vss *name, vss *val) { // Вытаскивает MimeHeader
*val=getMimeLine(src);
*name=vssGetTillStr(val,":",1,1);
vssTrim(name); vssTrim(val);
return name->len>0;
}

int vssFindMimeHeader(vss *src,uchar *name, vss *val) { // Ищет и заполняет val по имени хедера
vss SRC=*src,n=vssCreate(name,-1),N1,V1; // CopyOf
while(SRC.len>0) {
    vssGetMimeHeader(&SRC,&N1,&V1); // Get OneByOne
    if (N1.len==n.len && strnicmp(N1.data,n.data,n.len)==0) { // YES!
        *val=V1; return 1;
        }
    }
return 0; // Not Found
}

vss getMimeField(vss *src, char *name, int nl) {
vss out; int ch; uchar *s = src->data; int len = src->len;
if (nl<0) nl = strlen(name);
while(len>nl) if (memcmp(s,name,nl)==0) { // May Be a field ???
    s+=nl; len-=nl;
    //printf("Looks like found <%s> starts <%s>\n",name,s);
    while(len>0 && *s<=32) { s++; len--;}
    if (len<=0 || *s!='=') break;
    s++; len--;
    while(len>0 && *s<=32) { s++; len--;}
    if (*s=='"' || *s=='\'') { // retult is a string
        ch = *s; s++; len--; out.data=s; out.len=0;
        while(len>0 && *s!=ch) { s++; len--; out.len++;}
        return out;
        }
    out.data=s; out.len=0; // collect simple value
    while(len>0 && !strchr("\r\n\t ;,='\"",*s)) { s++; len--; out.len++;}
    return out;
    } else if (*s=='"' || *s=='\'') { // skip a string
    ch = *s;
    s++; len--;
    while(len>0 && *s!=ch) { s++; len--;}
    if (len>0) { s++; len--;}
    } else { s++; len--;}
out.len=0; out.data=0; return out;
}

vss vssExtractFileName(uchar *s, int len) {
int i,found; vss out;
if (len<0) len=strlen(s); found=-1;
for(i=0;i<len;i++) if (s[i]=='/' || s[i]=='\\') found=i;
if (found>=0) { out.data=s+found+1; out.len=len-found-1; return out;}
out.data=s; out.len=len; return out;
}

vss vssGetReportPart(vss *D, int *typ) { // Extracts till '<? * >'
vss out;
if (D->len>4 && memcmp(D->data,"<?vs",4)==0) { // Ye - define till '?>'
    out = vssGetTillStr(D,"?>",2,1);
    out.data+=4; out.len-=4;
    vssTrim(&out);
    *typ=1;
    return out;
    }
*typ=0;
return vssGetTillStr(D,"<?vs",4,0); // do dont remove devider !!!
}

vss vssVSP(vss *src, vss *code) {
vss text;
code->len=0;
text = vssGetTillStr(src,"<%",2,0); // Берем текст который впереди
if (src->len) { // Значит - дальше есть место для кода !!!
  vssGetLen(src,2);
  *code = vssGetTillStr(src,"%>",2,1);
  }
return text; // Готово !!!
}

int vssLcmp(vss *src, char *name) {
int l = strlen(name);
if (src->len>=l && memcmp(src->data,name,l)==0) return l;
return 0;
}

int vsslcmp(vss *src, char *name) {
int l;
l = vssLcmp(src,name);
if (l) { src->data+=l; src->len-=l; return 1;}
return 0;
}


int vssEQ(vss *x, uchar *str) {
return x->len == strlen(str) && memcmp(x->data,str,x->len)==0;
}

int vssCMP(vss *n1,vss *n2) {
int r = n1->len - n2->len;
if (r) return r;
return strncmp(n1->data,n2->data,n1->len);
}

int vssvss(vss *str,vss *substr) { // check - if we have it as substr???
int i,l = str->len-substr->len;
for(i=0;i<=l;i++) {
    if (memcmp(str->data+i,substr->data,substr->len)==0) return i;
    }
return -1;
}

int vssHasOne(vss *str,vss patt,uchar *del) {
int cnt=0; vss p;
while(patt.len>0) {
    int code;
    p = vssGetTillStr(&patt,del,-1,1); // TillDel
    code = vssvss(str,&p);
    //printf("HAS ONE=%d on IF <%*.*s> IN <%*.*s>\n",code,VSS(p),VSS(*str));
    if (code>=0) return cnt;
    cnt++;
    }
return -1; // no !
}

vss vssGetWord(vss *src) {
int i; vss res;
while( (src->len>0) && (src->data[0]<=32) ) {src->len--; src->data++;}; // lTrim - skip spaces
// now - check if wwe have '"'
if ((src->len>0) && ((src->data[0]=='"') || (src->data[0]=='\'')))
     { for(i=1;i<src->len;i++) if (src->data[i]==src->data[0]) { i++; break;} } // collect string -)))
else for(i=0;(i<src->len)&&(src->data[i]>32);i++); // collect a word
res.data = src->data; res.len = i; src->data+=i; src->len-=i;
while(src->len>0 && src->data[0]<=32) {src->len--; src->data++;}; // lTrim - trim it again
return res;
}

uchar *vss2str(uchar *buf, int size, vss *v) {
size = v->len>=size?size-1:v->len;
strncpy(buf,v->data,size);
buf[size]=0;
return buf;
}



