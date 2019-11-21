#include "httpSrv.h"
#include "wsSrv.h"

//VS0OBJH(Socket);

void wsSrvDone(wsSrv *s) {} ; // destructor

VS0OBJ0(wsSrv,wsSrvDone);

wsSrv *wsSrvCreate() { // Создание соединения по умолчанию для http
wsSrv *srv;
srv = wsSrvNew();
TimeUpdate();
strcpy(srv->name,"wsSrv"); srv->logLevel = 6; // Debug !!!
strcpy(srv->srv.name,"ws"); srv->srv.logLevel = 6;  // LoggerName
//srv->keepAlive = 1; // Yes, keep it
srv->created = TimeNow;
//srv->readLimit.Limit = 1000000; // 1 mln в секунду -)))
//srv->maxPPS = 20; // По умолчанию столько запросов в секунду принимается -)))
 // Применяем настройки ini
return srv;
}

int wsSrvStep(wsSrv *srv) { // >0 on run
 return SocketPoolRun(&srv->srv);
}

int wsSrvProcess(wsSrv *srv) { // Dead loop till the end???
while(!aborted) {
  TimeUpdate(); // TimeNow & szTimeNow
  int cnt = SocketPoolRun(&srv->srv);
  //printf("SockPoolRun=%d time:%s\n",cnt,szTimeNow); msleep(1000);
  RunSleep(cnt); // Empty socket circle -)))
  if (srv->runTill && TimeNow>=srv->runTill) break; // Done???
  }
return 0;
}

#ifdef VOS_SSL
#include <openssl/sha.h>
#else
#include "sha1/sha1.h" // in vos/sha1
#endif

void SocketClearPool(Socket *sock) {
SocketPool *srv = sock->pool;
objAddRef(sock); // extra ref
//printf("die msg=%s\n",msg);
//if (sock->onDie) sock->onDie(sock,msg); // Alert it
//sock->state = sockNone; sock->connectStatus = connectNone; sock->modified = TimeNow;
if (srv) { // Unreg from a pool -> some spec destructors ???
    CLOG(srv,2,"Socket#%d handle:%d, (%s) removed\n",sock->N,sock->sock,sock->szip);
    //if (srv->onDie) srv->onDie(sock); // Call on close
    sock->pool = 0;
    //printf("Remove!!?? from pool=%p ref=%d?\n",srv,obj2head(sock)->ref);
    vecRemove((void***)&srv->sock,sock); // Remove from an queue
    //printf("Remove!\n");
    }
}

//#include "hexdump.c"

int wsFireOnMessage(Socket *sock,unsigned char *data,int len) {
wsSrv *ws = (void*)sock->parent;
 unsigned char back = data[len];
 data[len]=0;
 // printf("wsOnMessage String:<%s>",data);
 if (ws->onMessage) ws->onMessage(sock,data,len);
 data[len]=back;
 return 0;
}

enum { wsNormal=1000,wsNoPeer=1001,wsProtocolError=1002,wsDataError=1003};
#define wsMAXPACKET 1024

void wsSendClose(Socket *sock,int reason) {
unsigned char buf[4]={0x08,0x02, reason>>8, reason &0xFF };
SocketSend(sock,buf,4);
sock->dieOnSend=1;
printf("wsClose sock=%d with reason:%d\n",sock->sock,reason);
}

char *wsUnmask(unsigned char *mask,int l) {
unsigned char *d = mask+4; // data starts here
int i; for(i=0;i<l;i++) d[i]=d[i]^mask[i%4];
return d;
}

int onWebSocketPacket(unsigned char *data, int len, Socket *sock) {
if (len == 0 ) return 0;
//printf("len=%d data[0]=0x%x\n",len,data[0]);
hex_dump("wsRawData",data,len);
if (len<2) return 0; // no data here
int masked = data[0] >> 7;
if (!masked) { wsSendClose(sock,wsProtocolError); return len;} // data MUST be masked
int opcode = data[0]&0x7f;
if (opcode !=1 ) { wsSendClose(sock,wsProtocolError); return len;} ;// data MUST be text
int l = data[1] & 0x7F;
if (l==127) {  // entended len
     if ( len<2+4+4) return 0; // need ID+LEN+exlen[4]+mask[4]
     unsigned int  rlen = (data[2]<<24) | (data[3]<<16) | (data[4]<<8) || data[5] ; // real len
     if (rlen>wsMAXPACKET) {  wsSendClose(sock,wsProtocolError); return len;} ;// too big packet
     if ( len<2+4+4+rlen) return 0; // not ready yet
     wsFireOnMessage( sock, wsUnmask(data+6,rlen), rlen);
     return 2+4+4+rlen; //
     }
  else
if (l==126) { // extended len 2bytes in len
     if ( len<4+4) return 0; // need ID+LEN+exlen[2]+mask[4]
     int  rlen = (data[2]<<8) | data[3]; // real len
     //printf("realen=%d\n",rlen);
     if ( len<4+4+rlen) return 0; // not ready yet
     // printf("realen=%d packetlen=%d\n",rlen,len);
     wsFireOnMessage( sock, wsUnmask(data+4,rlen),rlen);
     return 4+4+rlen; //
     }
  else {
  if ( len<l+2+4 ) return 0;
  wsFireOnMessage( sock, wsUnmask(data+2,l),l);
  return l+2+4; // data ready
  }
  /*if ( (!masked) && (len>=l+2)) { // ok ready packet
     //int i;
     //unsigned char *mask = data+2;
     unsigned char *d = data+2;
     //for(i=0;i<l;i++) d[i]=d[i]^mask[i%4];
      wsFireOnMessage(sock,d,l);
     //printf("wsString:<%*.*s>",l,l,d);
     return l+2; // data ready
     }
    }
*/
hexdump("WebSocket -> wsUnknownData",data,len); // never reach it?
return 0;
} ; // to do

int wsSrvUpgrade(wsSrv *ws, Socket *sock, vssHttp *req) {
char buf[1024],buf2[512];
//vss reqID = {0,0};
vss K; char b64[200];
//if (req && req->reqID.len>0) reqID=req->reqID;
//if (len<0) len = strlen(data);
if (!vssFindMimeHeader(&req->H,"Sec-WebSocket-Key",&K)) {
  CLOG(ws,2,"new webSocket declined, no Sec-WebSocket-Key\n");
  return 0;
  }
//printf("Key=%*.*s\n",K.len,K.len,K.data);
vss2str(buf,40,&K);
//strcpy(buf,"dGhlIHNhbXBsZSBub25jZQ==");
strcat(buf,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
unsigned char hash[SHA_DIGEST_LENGTH];
//printf("MakeIt to sha: %s\n",buf);
#ifdef VOS_SSL
SHA1(buf,strlen(buf), hash);
#else
SHA1(hash, buf,strlen(buf));
#endif

encode_base64(b64,hash,SHA_DIGEST_LENGTH);
//printf("b64: %s\n",b64);

snprintf(buf2,sizeof(buf2),"HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade: websocket\r\n"
"Connection: Upgrade\r\n"
//"Sec-WebSocket-Version: 13\r\n"
"Sec-WebSocket-Accept: %s\r\n"
//"Sec-WebSocket-Protocol: chat\r\n" --chrome doesnt love this!!!
"\r\n",b64);

SocketSend(sock,buf2,-1); // Add a header
//strCat(&sock->out,data,len); // Push it & Forget???
//printf("TOSEND:%s\n",buf2);

//printf(" ===== ref1=%d\n",obj2head(sock)->ref);

sock->state = sockSend;
SocketClearPool(sock); // extra ref here
Socket2Pool(sock,&ws->srv); // add to ws pool
CLOG(ws,3,"new webSocket#%d from '%s', accepted\n",sock->N,sock->szip);
sock->parent = ws;
sock->checkPacket = onWebSocketPacket; // When new packet here...
if (ws->readLimit.Limit) sock->readPacket = &ws->readLimit; // SetLimiter here

//printf(" ====== ref2=%d\n",obj2head(sock)->ref);
//objClear(&sock); // remove extra ref
// Wait???
return 1;
}

int utf8_peek(char *d,char *s,int l);

int utf8_valid(char *d,char *s, int l) { // validates source as utf8 and fills ok chars
int r = 0;
if (l<0) l = strlen(s);
while(l>0) {
  char U[6];
  int len = utf8_peek(U,s,l);
 //printf("LEN:%d for %02x %02x REST:%d\n",len,s[0],s[1],l);
  if (len<0) { s++; l--; continue; } // just skip invalid
  if (d) { memcpy(d,s,len); d+=len;}
  s+=len; r+=len; l-=len; // ok, check next
  }
if (d) *d=0;
return r;
}

int wsPutStr(Socket *sock, char *data,int len) {
//char ch=0;
if (len<0) len = strlen(data);
int ulen = utf8_valid(0,data,len);
if (ulen!=len) { // not utf8 - send only 254 first valid
   char buf[256];
   printf("ws-correcting len=%d to ulen=%d\n",len,ulen);
   if (len>sizeof(buf)-1) len=sizeof(buf)-1;
   len = utf8_valid(buf,data,len); data=buf;
   }
 // clear, only valid data here
  //char h[]={0x81,0x05,0x48,0x65,0x6c,0x6c,0x6f}; strCat(&sock->out,h,7); //data,len);
//if (len>=125) len=125; // ZUZU - trim as is
if (len<=125) {
    char h[2]={0x81,len};
    SocketSend(sock,h,2); SocketSend(sock,data,len);
   } else {
if (len>65535) len=65535;
   char h[4]={0x81, 126, len>>8, len&0xFF};
    SocketSend(sock,h,4); SocketSend(sock,data,len);



}
/*
if (len<0) len = strlen(data);
ch = 0; strCat(&sock->out,&ch,1); //zero
strCat(&sock->out,data,len);
ch=255; strCat(&sock->out,&ch,1); //end of data
*/
sock->state = sockSend;
return 1; // ok
//SocketS
}

int wsBroadcast(wsSrv *srv, char *msg,int len) { // send message to all
int i; //,cnt=0;
SocketPool *sp = &srv->srv;
if (len<0) len = strlen(msg);
for(i=0;i<arrLength(sp->sock);i++) wsPutStr(sp->sock[i],msg,len);
return i;
}

