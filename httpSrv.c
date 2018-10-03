#include "httpSrv.h"
#include <stdarg.h>
#include <fcntl.h>

/*

  Декларации для http - server:
    1. мапирование файловых систем
    2. мапирование внешних точек доступа
    3. авторизация

*/

void httpSrvDone(httpSrv *srv) { // Destroy it

}

void SocketMapDone(SocketMap *map) { // Удаление единицы массива
strClear(&map->name);
}

vssHttp httpReq(uchar *data, int len) { // Move to VSS?
vssHttp h; vss R;
h.R = h.B = vssCreate(data,len);
h.H = vssGetTillStr(&h.B,"\r\n\r\n",4,1); // Split URL+Heads & Body
R = vssGetRow(&h.H); // extracts first row
h.M = vssGetTillStr(&R," ",1,1); // GET
h.U = vssGetTillStr(&R," ",1,1); // /index.html
h.V = R; // HTTP/1.1
h.args = h.U; h.page = h.file = vssGetTillStr(&h.args,"?",1,1);
if (!vssFindMimeHeader(&h.H,X_REQUEST_ID,&h.reqID)) { h.reqID.data=0; h.reqID.len=0;}; // NoHeader
h.keepAlive=-1; // undefined
if (vssFindMimeHeader(&h.H,"Connection",&R))
  h.keepAlive = (R.len>=10 && strnicmp(R.data,"keep-alive",10)==0);
return h;
}

int httpReady(char *s) { // zu - to utils???
char *eoh, *ct, *r=s; int hlen,clen,wait;
if (!s) return 0; // no yet
while(*r && *r<=32) r++;
eoh = strstr(r,"\r\n\r\n"); if (!eoh) return 0; // no "end of headers"
ct = strstr(r,"Content-Length:"); if (!ct || ct>eoh) return 4+(eoh-s);  // no Content-Length or after headers
hlen = 4+(eoh-s); clen = 0; sscanf(ct+15,"%d",&clen); // Define Header length & content length
wait = hlen+clen-strLength(s);
//printf("Here HttpReady hlen=%d clen=%d strLen=%d wait=%d\n%s",hlen,clen,strLength(s),wait,s);
if (strLength(s)>=hlen+clen) {
    //printf("%s\n",s);
    return hlen+clen;
    }
return 0; // Not ready yet...
}




VS0OBJ0(httpSrv,httpSrvDone);
VS0OBJ0(SocketMap,SocketMapDone);
VS0OBJ0(httpMime,0);


int SocketMapSortDo(SocketMap **sm1,SocketMap **sm2) {
return -strcmp(sm1[0]->name,sm2[0]->name); // Самые длинные - первые
}

SocketMap *httpSrvAddMap(httpSrv *srv, uchar *Name, void *proc, void *Data) { // Такой вот хендлер
SocketMap *sm;
sm = SocketMapNew();
sm->name = Name;
sm->page = vssCreate(Name,-1);
//sm->data = objAddRef(data);
sm->onRequest = proc;
sm->data = Data;
// now - push it
SocketMapPush(&srv->map,sm);
qsort(srv->map,arrLength(srv->map),sizeof(void*),(void*)SocketMapSortDo);
//SocketMapClear(&sm);
return sm; // Return Self
}

int getFileData(char *filename,uchar **out) { // Тупо - пытаемся загрузить файл, если получится - отсылаем, инача - NULL
int file,len; uchar *str;
file = open(filename,O_RDONLY | O_BINARY,0);
if (file<=0) return -1;
len = filelength(file);
if (len<=0) return -2;
str = strCat(out,0,len); // Add a data
if (!str) { close(file); return -1; }
if (read(file,str,len)!=len) { close(file); return -1;};
close(file);
return len;
}

SocketMap *SocketMapFind(SocketMap **map, vss *page) { // Check for a map -- ???
int i;
for(i=arrLength(map)-1;i>=0;i--,map++) {
    SocketMap *m = map[0];
    //printf("i=%d\n",i);
    int l  = m->page.len;
    if (l<=page->len && memcmp(m->page.data,page->data,l)==0) {
     //printf("FFF\n");
     page->data+=l; page->len-=l; // rest of
     //printf("Found name=%s file='%*.*s' data=%s\n",m->name,VSS(*page),m->data);
     return m; // found
     }
    }
//printf("mapNotFound\n");
return 0; // not found
}

/*

vssHttp httpReq(uchar *data, int len) {
vssHttp h; vss R;
h.R = h.B = vssCreate(data,len);
h.H = vssGetTillStr(&h.B,"\r\n\r\n",4,1); // Split URL+Heads & Body
R = vssGetRow(&h.H); // extracts first row
h.M = vssGetTillStr(&R," ",1,1); // GET
h.U = vssGetTillStr(&R," ",1,1); // /index.html
h.V = R; // HTTP/1.1
h.args = h.U; h.page = h.file = vssGetTillStr(&h.args,"?",1,1);
return h;
}
*/
httpSrv *httpSrvCreate(char *ini) { // Создание соединения по умолчанию для http
httpSrv *srv;
srv = httpSrvNew();
TimeUpdate();
strcpy(srv->name,"httpSrv"); srv->logLevel = 6; // Debug !!!
strcpy(srv->srv.name,"http"); srv->srv.logLevel = 6;  // LoggerName
srv->keepAlive = 1; // Yes, keep it
srv->created = TimeNow;
srv->readLimit.Limit = 1000000; // 1 mln в секунду -)))
//srv->maxPPS = 20; // По умолчанию столько запросов в секунду принимается -)))
 // Применяем настройки ini
return srv;
}


SocketMap *httpSrvAddFS1(httpSrv *srv,uchar *url,uchar *path) {
CLOG(srv,2,"...adding FS map url:'%s' -> fs_path:'%s'\n",url,path);
return httpSrvAddMap(srv,strNew(url,-1), onHttpFS,strNew(path,-1));
}

int httpSrvAddFS(httpSrv *srv,uchar *url,...) { // Adding some FS mappings
uchar *path; int cnt=0;
va_list va;
va_start(va,url);
while(url) {
    path = va_arg(va,char*); // gets a next
    if (!path) break; // error!!!
    // ok - add it!!!
    httpSrvAddFS1(srv,url,path);
    url = va_arg(va,char*); // next one
    cnt++;
    }
va_end(va);
return cnt;
}
/*

int httpReady(char *s) {
char *eoh, *ct, *r=s; int hlen,clen,wait;
if (!s) return 0; // no yet
while(*r && *r<=32) r++;
eoh = strstr(r,"\r\n\r\n"); if (!eoh) return 0; // no "end of headers"
ct = strstr(r,"Content-Length:"); if (!ct || ct>eoh) return 4+(eoh-s);  // no Content-Length or after headers
hlen = 4+(eoh-s); clen = 0; sscanf(ct+15,"%d",&clen); // Define Header length & content length
wait = hlen+clen-strLength(s);
//printf("Here HttpReady hlen=%d clen=%d strLen=%d wait=%d\n%s",hlen,clen,strLength(s),wait,s);
if (strLength(s)>=hlen+clen) {
    //printf("%s\n",s);
    return hlen+clen;
    }
return 0; // Not ready yet...
}
*/



int SocketSendHttpCode(Socket *sock, vssHttp *req, char *code, uchar *data, int len) {
char buf[1024];
vss reqID = {0,0};
if (req && req->reqID.len>0) reqID=req->reqID;
if (len<0) len = strlen(data);
snprintf(buf,sizeof(buf),"HTTP/1.1 %s\r\nConnection: %s\r\n%s: %*.*s\r\nContent-Length: %d\r\n\r\n",code,sock->dieOnSend?"close":"Keep-Alive",
    X_REQUEST_ID,VSS(reqID),len);
SocketSend(sock,data,len);
sock->state = sockSend;
// Wait???
return 1;
}



int SocketSendHttp(Socket *sock, vssHttp *req, uchar *data, int len) {
return SocketSendHttpCode(sock,req,"200 OK",data,len);
}

int SocketPrintHttp(Socket *sock,vssHttp *req, uchar *fmt,...) { // Simple Print Here
char buf[1024];
BUF_FMT(buf,fmt);
return SocketSendHttp(sock,req,buf,strlen(buf));
}

#ifdef WIN32
int httpGetDirList(vss url,char *dir,uchar **out) {
char buf[1024];
WIN32_FIND_DATA fd;
HANDLE fh;
snprintf(buf,sizeof(buf),"%s/*.*",dir); // Copy Dir
fh = FindFirstFile(buf,&fd);
if (fh==INVALID_HANDLE_VALUE) return -1; // Fail
while(FindNextFile(fh,&fd)) {
    char *name = fd.cFileName;
    if (name[0]=='.') continue; // Never show
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        snprintf(buf,sizeof(buf),"<a href=\"%*.*s%s/\"  >%s/</a><br>\n",VSS(url),name,name);
        } else {
        snprintf(buf,sizeof(buf),"<a href=\"%*.*s%s\" title=\"size:%d\" >%s</a><br>\n",VSS(url),name,(int)fd.nFileSizeLow,name);
        }
    strCat(out,buf,-1);
        /*
    f->sz = fd.nFileSizeHigh;
    f->sz = (f->sz << 32) + fd.nFileSizeLow; // OK?
    f->created = fileLocalTime(&fd.ftCreationTime);
    f->modified = fileLocalTime(&fd.ftLastWriteTime);
    */
    }
FindClose(fh);
return 1; // OK
}
#else
#include <dirent.h>
int httpGetDirList(vss url,char *dir,uchar **out) {
char buf[1024];
//strCat(out,"linux unimplemented yet",-1);
DIR *dirHandle;
snprintf(buf,sizeof(buf),"%s",dir); // Copy Dir
dirHandle = opendir(buf);
if (!dirHandle) {
    printf("!!!opendir '%s' failed!!!",buf);
    return -1;
    }
struct dirent *fd;
while(1) {
    fd=readdir(dirHandle); if (!fd) break;
    char *name = fd->d_name;
    if (name[0]=='.') continue; // Never show
    struct stat st;
    uchar fname[1024];
    snprintf(fname,sizeof(fname),"%s/%s",dir,name);
    if (lstat(fname,&st)!=0) continue; // error on stat??? - link follows
    if (st.st_mode & S_IFDIR) {
        snprintf(buf,sizeof(buf),"<a href=\"%*.*s%s/\"  >%s/</a><br>\n",VSS(url),name,name);
        } else {
        snprintf(buf,sizeof(buf),"<a href=\"%*.*s%s\" title=\"size:%ld\" >%s</a><br>\n",VSS(url),name,st.st_size,name);
        }
    strCat(out,buf,-1);
    }
closedir(dirHandle);
return 1;
}
#endif


int onHttpFS(Socket *sock, vssHttp *req, SocketMap *map) { // Обработка запросов указанной страницы
char szPath[MAX_PATH];
httpSrv *srv = (void*)sock->pool;
int len;
snprintf(szPath,sizeof(szPath),"%s/%*.*s",map->data,VSS(req->page));
//printf("Try file <%s>\n",szPath);
len = strlen(szPath); strSetLength(&srv->buf,0);
if (len>0 && szPath[len-1]=='/') { // is Dir!!!
    if (httpGetDirList(req->U,szPath,&srv->buf)>0) { // List Generated
        len = strLength(srv->buf);
        CLOG(srv,3,"+OK dirList '%s' generated for '%s', (%d bytes)\n",szPath,sock->szip,len);
        SocketSendHttp(sock,req,srv->buf,len);
        } else {
        CLOG(srv,2,"-FAIL dirList '%s' generation for %s'\n",szPath,sock->szip);
        SocketPrintHttp(sock,req,"Wrong directory");
        }
    return 1; // DONE
    }
httpMime *mime; vss m = srv->defmime;
mime = httpSrvGetMime(srv,req->page); if (mime) m = mime->mime;
len  = getFileData(szPath,&srv->buf); //req->page);
if (len<=0) {
    SocketPrintHttp(sock,&srv->req,"File %*.*s not found on a server, code:%d",VSS(req->page),len);
    CLOG(srv,2,"-FAIL url '%*.*s', getfile '%s' for '%s'\n",VSS(req->page),szPath,sock->szip);
    return 1;
    }
CLOG(srv,3," sent '%*.*s', %d bytes for %s\n",VSS(req->U),len,sock->szip);
snprintf(szPath,sizeof(szPath),"HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %*.*s\r\nConnection: %s\r\n\r\n",len,VSS(m),
   sock->dieOnSend?"close":"Keep-Alive");
CLOG(srv,6,"new httpResponce#%d/%d headers '%s'\n",sock->N,sock->recvNo,szPath);
SocketSend(sock,szPath,-1);
SocketSend(sock,srv->buf,len); // Push it & Forget???
return 1; // -- process
}


int basicAuth_getUserPass(vss *heads,char *buf,int size) { // fill user-password buffer
vss A={0,0};
char *p;
int ok;
if (!vssFindMimeHeader(heads,"Authorization",&A)) return 0;
vss2str(buf,size,&A);
//   printf("Auth: %s\n", buf);
p = strstr(buf,"Basic");
if (!p) return 0;
p+=5; while(*p && p==' ') p++;
memmove(buf,p,strlen(p)+1); p=buf; // remove "basic"
decode_base64(p,p,strlen(p));
char *u=p; p=strchr(u,':'); if (!p) return 0;
//*p=0;
//printf("User=%s Pass=%s\n",u,p+1);
return 1; // no auth yet
}

int onHttpClientPacket(uchar *data,int len, Socket *sock) { // CHeck - if packet ready???
//vssHttp req;
len = httpReady(data); if (len<=0) return len; // Process protocol error or not ready
httpSrv *srv = (void*)sock->pool; // Есть, у меня есть счетчик пакетов - прямо на сокете???
CLOG(srv,4,"new httpRequest#%d/%d client '%s'\n",sock->N,sock->recvNo,sock->szip);
CLOG(srv,6,"new httpRequest#%d/%d requestBody '%s'\n",sock->N,sock->recvNo,data);
// Сначала - пытаемся найти мапу. Для этого - нужно разобрать запрос
if (!srv->keepAlive) sock->dieOnSend = 1;
srv->req = httpReq(data,len); // Быстро разбираем запрос
#ifdef HTTPSRV_AUTH
char buf[512];
int  auth_ok = basicAuth_getUserPass(&srv->req.H,buf,sizeof(buf));
if  (auth_ok && srv->auth) auth_ok = srv->auth(buf,srv);
srv->userId = auth_ok; // copy
if (auth_ok<=0) { // unauth
   if (!srv->realm) srv->realm="";
   snprintf(buf,sizeof(buf),"401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"%s\"",srv->realm);
   SocketSendHttpCode(sock,&srv->req,buf, 0,0 );
   return len;
   }
#endif
SocketMap *map;
map =SocketMapFind(srv->map, &srv->req.page);
if (!map) {
    CLOG(srv,3,"reqest '%*.*s' - not mapped %d/%d,client '%s'\n",
           VSS(srv->req.page), sock->N,sock->recvNo,sock->szip);
    SocketPrintHttp(sock,&srv->req,"Request#%d '%*.*s' Unmapped. ServerTime:%s",sock->recvNo,VSS(srv->req.page),szTimeNow); // GOOD???
    } else { // Map Found !!!
    map->onRequest(sock,&srv->req,map); // ProcessThis by a Map
    //SocketPrintHttp(sock,"Request#%d Accepted, MAP=%s, ServerTime:%s",sock->recvNo,map->name, szTimeNow); // GOOD???
    }
return len; // remove from a read buffer & process...
}


int onHttpClientConnect(Socket *lsock, int handle, int ip) {
Socket *sock;
httpSrv *srv = (void*)lsock->pool; // Here My SocketServer ???
sock = SocketPoolAccept(&srv->srv,handle,ip);
CLOG(srv,3,"new connect#%d from '%s', accepted\n",sock->N,sock->szip);
sock->checkPacket = onHttpClientPacket; // When new packet here...
if (srv->readLimit.Limit) sock->readPacket = &srv->readLimit; // SetLimiter here (if any?)
return 1; // Everything is done...
}

Socket *httpSrvListen(httpSrv *srv,int port) {
Socket *sock = SocketNew();
if (SocketListen(sock,port)<=0) {
    CLOG(srv,1,"-fail listen port %d",port);
    SocketClear(&sock); return 0; } // Fail Listen a socket???
CLOG(srv,2,"+listener started on port#%d",port);
sock->onConnect = onHttpClientConnect;
Socket2Pool(sock,&srv->srv);
return sock;
}


// -- some mimes

int httpSrvAddMimes(httpSrv *srv,char *mimes) {
vss Mimes;
strCat(&srv->mimes,"&",-1); strCat(&srv->mimes,mimes,-1);
Mimes = vssCreate(srv->mimes,-1);
if (!srv->mime) srv->mime = httpMimeArray(10);
// Тупо приклеиваем строку и разбираем заново???
while(Mimes.len>0) {
    vss E,E1,M;
    M = vssGetTillStr(&Mimes,"&",1,1); vssTrim(&M); if (M.len==0) continue;
    E = vssGetTillStr(&M,"=",1,1); vssTrim(&E);
    while(E.len>0) {
        httpMime *m;
        E1=vssGetTillStr(&E,",",1,1); vssTrim(&E);
        if (E1.len>0) {
            m = httpMimeAdd(&srv->mime,0);
            m->ext = E1;
            m->mime = M;
            //printf("Adding ext:'%*.*s', mime:'%*.*s'\n",VSS(E1),VSS(M));
            }
        }
    }
return 1;
}

httpMime *httpSrvGetMime(httpSrv *srv,vss file) { // Gets from ext
int i;
for(i=arrLength(srv->mime)-1;i>=0;i--) {
    httpMime *mime = srv->mime+i; vss *m = &mime->ext;
    if (m->len <= file.len &&
       memcmp(file.data+file.len-m->len,m->data,m->len)==0) return mime;
    }
return 0;
}

int httpSrvProcess(httpSrv *srv) { // Dead loop till the end???
while(!aborted) {
  TimeUpdate(); // TimeNow & szTimeNow
  int cnt = SocketPoolRun(&srv->srv);
  //printf("SockPoolRun=%d time:%s\n",cnt,szTimeNow); msleep(1000);
  RunSleep(cnt); // Empty socket circle -)))
  if (srv->runTill && TimeNow>=srv->runTill) break; // Done???
  }
return 0;
}


/// --- httpClient declarations

void httpClientClearCommands(httpClient *cli,int res_code) {
while(arrLength(cli->cmd)>0) { // Anyway - remove them
        httpCmd *cmd = cli->cmd;
        cmd->res_code = httpDisconnect; // -1
        if (cmd->onDone) cmd->onDone(cmd);
        arrDelN((void*)&cli->cmd,0,1);
        }
}

void httpClientDone(httpClient *cli) {
httpClientClearCommands(cli,httpDisconnect); // if have smth
SocketDone(&cli->sock); // Чистим сокет
httpCmdClear(&cli->cmd); // Очищаем массив
}

void httpCmdDone(httpCmd *cmd) { // Remove command from a list?
int (*onDone)()=cmd->onDone;
cmd->onDone=0;
if (onDone) onDone(cmd);
}

VS0OBJ0(httpClient,httpClientDone);
VS0OBJ0(httpCmd,httpCmdDone);

int httpClientRun(httpClient *cli) { // Функция обработки данного httpClient
int cnt = 0;
//printf("httpClientRun begin...\n");
if (SocketRun(&cli->sock)) cnt++; // Если отработал сокет более ничего не делаем?
//printf("CNT=%d\n",cnt);
if (arrLength(cli->cmd) && cli->sock.state<=0) { // We Have a job - but disconnected !!!
    //printf("Run1\n");
    if (cli->onDisconnect) cli->onDisconnect(cli); // Need reconnect now !!!
       else httpClientClearCommands(cli,httpDisconnect); // default - clear all commands???
    //printf("Done&dun again?\n");
    }
// Do timeouts
return cnt;
}


int onHttpClientRequestReplay(uchar *data,int len,httpClient *cli) { // Нотификация сокета о готовности указанных данных
httpCmd *cmd = 0;
len = httpReady(data); if (len<=0) return len; // Process protocol error or not ready
//printf("HttpClient -> packetReady %*.*s!\n",len,len,data);
cli->res = httpReq(data,len); // Разбираем запрос
if (cli->res.reqID.len>0) { // RequestID supported by server -)))
    int i,n = atoi(cli->res.reqID.data); // get By N
    for(i=0;i<arrLength(cli->cmd);i++) if (cli->cmd[i].n==n) {cmd=cli->cmd+i; break;}
    } else { // Pooling, first send - first replied
    if (arrLength(cli->cmd)>0) cmd = cli->cmd; // First One
    }
CLOG(cli,6,"HttpClientRequestReply: <%*.*s>\n",len,len,data);
if (!cmd) {
    CLOG(cli,1,"UNMAPPED RECV(%d bytes): <%*.*s> <%*.*s> <%*.*s> ReqId:(Req:%d,Serv:<%*.*s>)\n",
      len,VSS(cli->res.M),VSS(cli->res.U),VSS(cli->res.V),cmd?cmd->n:-1,VSS(cli->res.reqID));
    // ZU - abort???
    } else {
    CLOG(cli,4,"RECV(%d bytes): <%*.*s> <%*.*s> <%*.*s> ReqId:(Client:%d,Server:\"%*.*s\")\n",
    len,VSS(cli->res.M),VSS(cli->res.U),VSS(cli->res.V),cmd?cmd->n:-1,VSS(cli->res.reqID));
    int pos = cmd-cli->cmd;
    if (cmd->onDone) cmd->onDone(cmd); // Call It
    arrDelN((void*)&cli->cmd,pos,1); // RemoveIt
    //printf("NewArrLength:%d\n",arrLength(cli->cmd));
    }
return len;
}

void* strcatf(uchar *str,uchar *fmt,...) {
uchar buf[1024];
BUF_FMT(buf,fmt);
return strcat(str,buf);
}

int httpClientInit(httpClient *cli, uchar *cs,uchar *Proxy) { // Инитим клиента...
uchar szcs[sizeof(cli->cs)],proxy[sizeof(cli->proxy)],*u,*p,*h,buf[1024];
int defport=80;
cli->sock.checkPacket = onHttpClientRequestReplay; // Чиатем по пакетам
strNcpy(szcs,cs); cs=szcs; strNcpy(cli->cs,cs);// Copy It Here
if (!Proxy) Proxy="";
strNcpy(proxy,Proxy); Proxy=proxy; strNcpy(cli->proxy,proxy); // Copy It Here
u=p=0; cli->host = cli->cs; cli->Heads[0]=0; // No Authority ???
h = strchr(szcs,'@'); if (h) { // EmptyUser&Pass
    *h=0; h++; cli->host=strchr(cli->cs,'@')+1; // the same???
    u=szcs; p = strchr(u,'/'); if (p) *p=':';
    } else {
    h = szcs; u=p=0;
    }
if (u) {
    encode_base64(buf,u,strlen(u));
    strcatf(cli->Heads,"Authorization: Basic %s\r\n",buf); // Extra Adds
    }
cli->srv = cli->host; // If No Proxy
if (proxy[0]) { // Proxy defined !!!
    defport = 3128;
    cli->srv=cli->proxy; u=0;
    h = strchr(proxy,'@'); if (h) { // User-pass???
        *h=0; h++; cli->srv=strchr(cli->proxy,'@')+1;
        u = proxy; p = strchr(u,'/'); if (p) *p=':';
        }
    //strcatf(cli->Heads,"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9) Gecko/2008051206 Firefox/3.0\r\n");
    //strcatf(cli->Heads,"Accept:	text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    //strcatf(cli->Heads,"Accept-Language: en-us,en;q=0.5\r\n");
    //strcatf(cli->Heads,"Accept-Encoding: gzip,deflate\r\n");
    //strcatf(cli->Heads,"Accept-Charset:	ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n");
    //strcatf(cli->Heads,"Keep-Alive:	300\r\n");
    strcatf(cli->Heads,"Proxy-Connection: keep-alive\r\n");
    if (u) {
        encode_base64(buf,u,strlen(u));
        strcatf(cli->Heads,"Proxy-Authorization: Basic %s\r\n",buf);
        }
    } else {
    strcatf(cli->Heads,"Connection: keep-alive\r\n");
    }
cli->page="/";
u = strchr(cli->host,'/'); if (u) { *u=0; cli->page=u+1; }
// ok - now ConnectNow???
if (socketConnectNow(&cli->sock,cli->srv,defport)<=0) {
    CLOG(cli,1,"-httpClient tcp-connect to '%s' failed (defport:%d)",cli->srv,defport);
    return 0;
    }
CLOG(cli,2,"+httpClient connected with '%s' OK, heads: %s",cli->srv,cli->Heads);
return 1;
}


httpCmd *httpClientPost(httpClient *cli, uchar *url, uchar *data, int len) { // Flash a get string???
Socket *sock = &cli->sock;
int cmdn = ++cli->num, ok; // New command num
char *method="POST";
httpCmd *cmd;
char buf[2048];
if (!data) {  method="GET"; data=""; len=0; }
if (len<0) len  = strlen(data);
if (!url) url="";
while (*url=='/') url++; // Already has it -)))
//printf("Sendf?\n");
if (cli->proxy[0]) { // ProxyConnection???
    snprintf(buf,sizeof(buf),"%s http://%s%s%s HTTP/1.1\r\nHost: %s\r\n%s\r\n",
   method,cli->host,cli->page,url,cli->host,cli->Heads);
    }
else snprintf(buf,sizeof(buf),"%s /%s%s HTTP/1.1\r\nHost: %s\r\n%s: %d\r\n%sContent-Length: %d\r\n\r\n",
    method,cli->page,url,cli->host,X_REQUEST_ID,cmdn,cli->Heads,len);
ok  = SocketSend(sock,buf,strlen(buf));
if (ok && len) ok = SocketSend(sock,data,len);
if (!ok) return 0; // Failed?
//printf("Cmd?\n");
cmd = httpCmdAdd(&cli->cmd,0);
cmd->n = cmdn;
cmd->cli = cli; // Set A client
cmd->onDone = cli->onReply; // default handler here
CLOG(cli,5,"SEND req:%d Http:%s",cmd->n,buf);
//printf("OK - send get, cmdn=%d!\n",cmd->n);
return cmd;
}

httpCmd *httpClientGet(httpClient *cli, uchar *url) { // Flash a get string???
return httpClientPost(cli,url,0,0);
}
