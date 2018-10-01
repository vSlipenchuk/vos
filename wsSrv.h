#ifndef WSSRV_H_INCLUDED
#define WSSRV_H_INCLUDED

#include "sock.h"
#include "httpSrv.h"

typedef struct _wsSrv {
    SocketPool srv; // Тут все сокеты (включая слушателя)
    char name[14]; int logLevel; logger *log; // Logging
    //SocketMap **map; // Залинкованные URL (файловые и программные)
    //vssHttp req; // Текущий запрос на обработку - ???
    //uchar *index; // Кешированный индекс (отдается мгновенно - или сделать map?)
    //uchar *buf; // Временный файл (для закачки файлов)
    //uchar *mimes;
    //httpMime *mime; // Пассивная строка и собственно - разобранные маймы для быстрого поиска
    Counter readLimit; // Limiter for incoming counts
    //SocketPool cli; // Клиенты - для редиректа???
    //vss defmime; // DefaultMime for a page ???
    //int keepAlive; // Disconenct after send???
    time_t created; // When it has beed created -)))
    time_t runTill;
    void *handle; // any user defined handle
#ifdef HTTPSRV_AUTH
    char *realm; // report on 401 Unauthorized
    int  (*auth)(char *UserPass, struct _httpSrv *); // user:password, func must return >0 on success
    int  userId; // last auth result (app must copy it)
#endif
    int (*onMessage)(Socket *sock, unsigned char *msg, int len);
    } wsSrv;

VS0OBJH(wsSrv); // Общие декларациии запуска сервиса ...
wsSrv   *wsSrvCreate(); // Создание сервера webSockets

int wsSrvUpgrade(wsSrv *ws, Socket *sock, vssHttp *req); // get HTTP socket & Upgrade connection
int wsSrvStep(wsSrv *srv); // >0 on run

int wsSrvProcess(wsSrv *srv) ; // LOOP till aborted

// send string
int wsPutStr(Socket *sock, char *data,int len);



#endif // WSSRV_H_INCLUDED
