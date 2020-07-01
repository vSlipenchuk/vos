#ifndef SOCK_H
#define SOCK_H

#include "vos.h"
#include "vs0.h"
#include "vss.h"
#include "logger.h"
#include "vtypes.h"

//#include "counter.h"

enum {
    sockNone = 0, // Пустое состояние. Готово к удалению - если что -)))
    sockConnected = 1, // Готовый, соединенный сокет...
    sockListen = 2, // Находится в режиме слушания -)
    sockRead = 3, // С данного сокета нужно читать -)
    sockSend = 4, // sending a data -)))
    sockBusy = 5, // Running execution on external thread -)))
    sockKillMe = -8,
    };


enum {
    connectNone = 0, // Socket Not connected
    connectTCP = 1,  // Socket connected via TCP
    connectSSL = 2,  // SSL handShare OK
    connectApp = 3   // App connected OK
    };

typedef struct { // Это будет структура - сокет
    int sock; // Handle -))
    int state; // Текущее состояние сокета. Асинхронность - тоже сюда???
    uchar *out,*in; // Вход и выход - потоки, внутри которых лежат готовые данные на отправку
    void *ssl; // Если есть фильтр - SSL engine - то это тут все -)))
    int N; // Последовательный номер соединения
    int recvNo,recvBytes; // Номер последовательного запроса и количество принятых байтов
    int (*onConnect)(); // Функция - оповещение об осуществлении коннекта ...
    int (*onDataSent)();  // When data done
    int (*checkPacket)(); // Собственная функция готовности пакета - что с ним делать?
    int (*onDie)(); // Когда сокет умирает (дисконнект)
    int writable,readable; // Transport flags - if we can read or write smth???
    Counter readCounter,writeBytes; // Счетчик чтения -)))
    Counter *readPacket; // Лимит на чтение
    int readPackets,readBytes; // Счетчики приема отсылки пакетов
    // -- ограничение чтения
    int maxBytes,Bytes0,Bytes1,Time0; // Если Time0>=now то можно считать maxByte*2
    int connectStatus; // None, TCP, SSL, App
    int connected; // time when it was connected -)))
    int  dieOnSend; // Флаг - отбивать соединения или нет после отсылки ответа???
    char szip[16]; // ip address
    //time_t stopReadTill; // флаг остановки чтения - для того, чтоб не забирать в буфер из потока...
    time_t modified; // Last read or write time
    void *parent; // This is parent structure
    struct _SocketPool *pool; // if socket in a pool???
    char name[80]; int logLevel; void *log;
    char status[80]; // last line for text messages
    int   code; // status code (assoc)
    int   lineMode; // lineMode '\r\n' or '\r\n.\r\n'
    void *handle; // any use
    void *auth; // authinfo if need to atuhoriztions of session
    } Socket;

uchar *ip2szbuf(int ip,uchar *buf);
uchar *ip2sz(int ip);

VS0OBJH(Socket);
void SocketDone(Socket *sock);


typedef struct _SocketPool {
    Socket **sock; // Все сокеты, которые внутри пула... зачем они тут???
    char name[14]; int logLevel; // Name of a system & loglevel of it (for logf...)
    logger *log; // Must Be Here ???
    int reported; // TimeLastReported
    int seqN; // Последовательный номер соединения
    Socket* (*onAccept)(void *srv,int sock,int ip); // How to process connection ???
    int (*DataIsReady)(); // Check - if data is ready for a socket???
    int (*onDataReady)(); // Check - if we have data enougth ???
    int (*onDataSent)();  // When data done
    int (*onDie)(); // When Socket Die called (socket removed from a pool
    //SocketMap **map; // Вектор нужных мапов
    int report; // Дапп-репорт о состояниях сокетов???
    int requests; // Total requests processed
    int connects; // Total connects served
    void *handle; // не используется - только для АПП
    #ifdef VOS_SSL
    char *pem_file;
    #endif
    } SocketPool;

VS0OBJH(SocketPool);

int SocketRun(Socket *sock);
Socket *SocketPoolAccept(SocketPool *srv, int isock, int ip); // Автоприем соединение с добавлением в пул

int socketConnectNow(Socket *Sock, char *host, int defPort) ; // Синхронный коннект сокета
int SocketSendDataNow(Socket *sock, void *data, int len); // Adds data to out & try send() it now

int SocketSend(Socket *sock,void *data,int len);
int SocketSendf(Socket *sock,char *fmt,...);
int SocketPoolRun(SocketPool *p); // Запускает асинхронные отсылки - приемы для каждого сокета
//int socketConnect(Socket *Sock, char *host, int defPort);
int SocketListen(Socket *Sock, int port) ; // Создаем слушателя...
int SocketRecvHttp(Socket *sock);
int SocketDie(Socket *sock,char *msg); // Notify & close this socket
void Socket2Pool(Socket *sock,SocketPool *srv); // Add To Pool

/// httpClient.c

/// gearConnect

#define GEAR_CALL_TIMEOUT 1*180 /* Максимальное время выполнения операции */
#define GEAR_CALL_NOOP    30 /* отсылать запрос на Noop для периода молчания более чем */

enum { rNew,rRun,rReady,rDone,rTimeOut}; // Статусы реквестов...

typedef struct { // Асинхронная заявка на выполнение задания
    int N; // Номер запроса...
    int status,code; // Текущий статус запроса и код обработки (httpResponce)
    time_t sent,timeOut; // Время когда запрос отослан и время после которого считается превышеным...
    int (*onState)(/*req,connect*/);  void *handle; // Данные для асинхронной нотификации о запросе
    int (*onUI)(/*req,connect*/);  // Можно подвесить обработчик на UserInteract...
    } gearRequest;

typedef struct { // Дочерний клиент
    Socket sock;
    char name[24];int logLevel; void *log; // logger option
    int  usr; // Пользовательский ID - заполняется после getMyInfo
    uchar cs[80],Host[80],Auth[80]; // connect string...
    int reqN; gearRequest *req; // Счетчик заявок и последовательный массив...
    vss resp,heads,data,status,httpVer; // responce here Response,Heads
    int httpStatus; // last HTTP status
    Counter readPacket; // для статистики...
    int toNoop;  // timeOut for send NOOP (keep-alive requests)
    time_t checked,pinged; // Проверка тайм-аутов и живости отсылок на сервер
    uchar userName[80]; // gearUserName
    int eof; // Flag EndOfFile...
    } gearClient;

VS0OBJH(gearRequest);
VS0OBJH(gearClient);

int gearClientRun(gearClient *gc);

int gearClientInit(gearClient *gc, uchar *cs); // копирует данные и устанавливает физический TCP коннект...
gearRequest *gearClientRequest(gearClient *gc, uchar *url, uchar *data, int len); // послать запрос
gearRequest *gearClientReqByN(gearClient *gc,int reqN);
int gearCallf(gearClient *gc,uchar **data, uchar *fmt,...); // Синхронный вызов

// some ready functions
int httpReady(char *s);

// extender - opens a connector + pool...
int SocketListener(Socket *sock,int port,
   void *onConnect, void *checkPacket, void *onDie);  // makes a pool & connect it to listener -)))


#endif // SockH
