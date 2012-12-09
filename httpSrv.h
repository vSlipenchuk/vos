#ifndef HTTP_SRV_H
#define HTTP_SRV_H

#define X_REQUEST_ID "X-Request-ID"

#include "vos.h"
#include "sock.h"
#include "vss.h"

typedef struct { // Маппер используется для соответствия URL (или SOAP)
    uchar *name; // Точка линковки мапа
    uchar *data; // Произвольные данные
    vss page; // == Name для ускорения поиска
    int enabled; // Можно включать-выключать "на лету"
    int (*onRequest)(); // Сокет, Сам Мар - функция, которая вызывается
    } SocketMap;

VS0OBJH(SocketMap);

int httpReady(char *s);
int SocketSendHttp(Socket *sock, vssHttp *req, uchar *data, int len);
vssHttp httpReq(uchar *data, int len); // Разборщик

typedef struct {
    vss ext,mime;
    } httpMime;

typedef struct {
    SocketPool srv; // Тут все сокеты (включая слушателя)
    char name[14]; int logLevel; logger *log; // Logging
    SocketMap **map; // Залинкованные URL (файловые и программные)
    vssHttp req; // Текущий запрос на обработку - ???
    uchar *index; // Кешированный индекс (отдается мгновенно - или сделать map?)
    uchar *buf; // Временный файл (для закачки файлов)
    uchar *mimes;
    httpMime *mime; // Пассивная строка и собственно - разобранные маймы для быстрого поиска
    Counter readLimit; // Limiter for incoming counts
    SocketPool cli; // Клиенты - для редиректа???
    vss defmime; // DefaultMime for a page ???
    int keepAlive; // Disconenct after send???
    time_t created; // When it has beed created -)))
    time_t runTill;
    void *handle; // any user defined handle
    } httpSrv;


VS0OBJH(httpSrv); // Общие декларациии запуска сервиса ...
VS0OBJH(httpMime);

httpSrv   *httpSrvCreate(char *ini); // Создание сервера http
SocketMap *httpSrvAddMap(httpSrv *srv, uchar *Name, void *proc, void *Data); // Такой вот хендлер
SocketMap *httpSrvAddFS1(httpSrv *srv,uchar *url,uchar *path); // Add One FileMapping
SocketMap *httpSrvAddRedirect(httpSrv *srv,uchar *Name,uchar *Host); // Adds New Mapping for a host
int httpSrvAddFS(httpSrv *srv,uchar * url,...);
Socket *httpSrvListen(httpSrv *srv,int port); // Включить на прослушку

int SocketPrintHttp(Socket *sock, vssHttp *req,uchar *fmt,...) ; // Simple Print Here

int httpReady(char *s); // http - stopper detector

int onHttpFS(Socket *sock, vssHttp *req, SocketMap *map) ; // Default File Processor // Обработка запросов указанной страницы
int httpGetDirList(vss url,char *dir,uchar **out); // Default Dir Generator

int httpSrvAddMimes(httpSrv *srv,char *mimes); // Добавление маймов
httpMime *httpSrvGetMime(httpSrv *srv,vss file); // Поиск внутри майма

int httpSrvProcess(httpSrv *srv) ;// Dead loop till the end???

/// -- client declrations

enum {
    httpDisconnect = -1, // server disconnected while run command
    httpTimeOut = -2, // TimeOut while run a command
    };

typedef struct { // Команда для асинхронного исполнения
    int  n; // Последовательный номер (X-Request-Id)
    int  res_code; // ResultedCode
    int  (*onDone)(); // httpCmd *cmd, vssHttp *res, httpClient *cli); // Вызывается, когда команда выполнилась
    void  *handle; // Любой внешний указатель, связанный с данным запросом
    uchar *data; // Тут - данные произвольной длины... Умирают автоматически -)))
    void  *srcHandle; // Произвольный неиспользуемый указатель - прозрачный блин???
    time_t created; // Время создания запроса. Для ловли тайм-аутов???
    struct _httpClient *cli;
    char buf[512]; // Маленький статический буферочек для маленьких данных???
    } httpCmd;


typedef struct _httpClient { //
    Socket sock; // Единичный сокет для работы ..
    char name[80]; logger *log; int logLevel; // MyLogLevel here
    uchar cs[128],proxy[128]; // Default @connect string@ = user/pass@host:port
    uchar *host,*srv,*page; // ConnectHost & PagePrefix & Authority (Basic64)
    uchar Heads[1024]; // Add heads
    vssHttp res; // Текущий декодированный респо
    int num; // Счетчик команд (начинаем с 1)
    httpCmd *cmd; // Список команд
    int (*onReply)(); // When server replyies on it
    int (*onDisconnect)(); // When a disconnects from a server
    } httpClient;

VS0OBJH(httpCmd);
VS0OBJH(httpClient);

int httpClientInit(httpClient *cli, uchar *cs, uchar *Proxy) ; // Инитим клиента...
int httpTestMain(int npar,char **par); // Тестовый клиент
int httpClientRun(httpClient *cli) ; // Функция обработки данного httpClient

void httpClientClearCommands(httpClient *cli,int res_code);

httpCmd *httpClientPost(httpClient *cli, uchar *url, uchar *data, int len) ; // Flash a get string???
httpCmd *httpClientGet(httpClient *cli , uchar *url); // Примитивная строка получения запроса

/// -- zu - microHttp

int MicroHttpMain(int npar,char **par); // Запуск микро-хттп

#endif // HTTP_SRV_H

