/*

 Работа с ини-файлами (добавок к vss)  - файл грузится внешне,
 мы должны уметь забирать из него секции и параметры.

 Классно было бы уметь грузить настройки с разных мест.
 Для этого - должна быть определена структуа "как читать конфиг файл" -
 а именно - перечисление имен переменных, их типов и сдвижек относительно указанной структуры...
 Как бы это затеять? - очень похоже на описание филдов в статическом файле?? - просто ужжастно!!!

*/


#include "vss.h"
#include "vtypes.h"
#include "vs0.h" // For String create

int iniSectionName(vss row,vss *name) { // Checks - if row is a section name???
vss r;
r = vssGetRow(&row);
vssTrim(&r);
if (r.len>=2 && r.data[0]=='[' && r.data[r.len-1]==']') {
    *name = vssCreate(r.data+1,r.len-2);
    return 1; // Yes
    }
return 0;
}

int iniNextSection(vss *src, vss *name, vss *data) {
vss n;
data->data=0; data->len=0; name->data=0; name->len=0; // Empty
while(src->len>0) {
    if (iniSectionName(*src,&n)) { // New Section Begins
        //printf("INI N=%*.*s, data=%d\n",VSS(n),data->data);
        if (data->data) { // We already collecing -> done collect
            data->len = ptrShift(data->data,src->data); // Till Here -)))
            return 1; // OK
            }
        // no - are we not collecting -> fill a name
        *name=n;
        vssGetRow(src);
        data->data=src->data; // start collect
        //printf("Continue, rest=%d\n"
        continue;
        }
    // whell - it is not a section, just skip ???
    //printf("Nothing - just skip?\n");
    vssGetRow(src);
    }

// ok - have we collected any???
if (data->data) {
    data->len = ptrShift(data->data,src->data); // Till Here -)))
    return 1;
    }
return 0; // no more -)))
}

int iniFindSection0(vss src, vss name,vss *v) { // FindSectionByName
vss n; // Name&Body here
while(iniNextSection(&src,&n,v)) {
    //printf("INISECTION NAME=<%*.*s> BODY=<%*.*s>\n",VSS(n),VSS(b));
    if (vssCMP(&n,&name)==0) return 1;
    }
v->data=0; v->len=0;
return 0;
}
int iniFindSection(vss src, uchar *name,vss *v) { // FindSectionByName
return iniFindSection0(src,vssCreate(name,-1),v);
}

int iniFindValue(vss src, uchar *name, vss *v) {
vss r, n;
while(src.len>0) {
    r = vssGetRow(&src);
    n = vssGetTillStr(&r,"=",1,1);
    //printf("n=%*.*s\n",VSS(n));
    vssTrim(&n);
    if (vssEQ(&n,name)) { *v = r; vssTrim(v);
        // Внутри могут быть "комментарии"
         r = *v; *v = vssGetTillStr(&r,";",1,1);
         vssTrim(v);
        return 1;
        } // Found
    }
return 0; // Not Found
}

int vssTrimStr(vss *v) {
vssTrim(v);
if (v->len>0 && strchr("\"'",v->data[0])  && v->data[v->len-1]==v->data[0]) {
    v->data++; v->len-=2;
    vssTrim(v);
    return 1;
    }
return 0;
}

uchar *iniFindStringDef(vss sec, uchar *name, uchar *def) {
vss v;
if (iniFindValue(sec,name,&v)) {
    vssTrimStr(&v);
    return strNew(v.data,v.len);
    }
if (!def) return 0;
return strNew(def,-1);
}

int iniFindStringBuf(vss sec, uchar *name, uchar *buf,int size) {
vss v;
if (!iniFindValue(sec,name,&v)) return 0;
vssTrimStr(&v); if (v.len>=size) v.len=size-1;
memcpy(buf,v.data,v.len); buf[v.len]=0;
return 1;
}


int iniFindIntDef(vss sec, uchar *name, int def) {
vss v;
if (iniFindValue(sec,name,&v)) {
    if (!scanUINT(v.data,v.len,&def)) return def;
    }
return def;
}
