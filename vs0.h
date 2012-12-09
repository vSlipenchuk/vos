#ifndef VS0
#define VS0

#include "vtypes.h" /* общие типы uchar,byte */

#define CHUNK_STEP 2 /* Стратегия минимального увеличения памяти - удвоение буфера */

typedef void * Obj; // Это мой основной тип=объект

typedef struct { // Распределитель памяти (vsMem.c)
    void *handle; // Собственный хендел
    void* (*malloc)(int size);
    void* (*realloc)(void *p, int size);
    void  (*free)(void *p);
    char *name; // Static name of allocer
    int ref,count,size;  // Статистика -  Суммарное количество ссылок, занятых переменных и полный размер ...
    } memDisp; // Мемори Диспетчер

extern memDisp defMem; // Дефолтовый встроенный мемори диспетчер

enum objClassTyp {
    clsDynamic = 16, // Динамически созданный класс. Сечем ссылки внутри данного класса (по объектам);
    clsDefine  = 1<<8, // Тип конструирования - определение
    clsStruct  = 3<<8, clsClass  = clsStruct, // Типа структура
    clsUnion   = 4<<8, clsChoice = clsUnion,
    clsArray   = 8<<8,
    clsRef     = 32,  // Индикатор типа хранения - что сохранять - саму структуру или только ссылку на нее?
    //clsArray   = 1, // Класс - массив. cls->base - указывает на структуру массива;
    clsVector  = 2, // Вектор - массив указателей на структуры базового класса;
    //clsStruct  = 3, // Класс - структура, cls->base указывает на родительскую струтуру;
    //clsChoice  = 4, // Класс = надстройка над REF?
    clsFunctor = 5, // Класс - функция. Определена функция call() в классе.
    };


typedef struct packed { // Мета описатель класса
    int typ;     // Маска - см objClassTyp
    uchar *name;  // Имя - статическое для статических и динамическое для динамических
    int size1;   // единица объема для инстанса класса
    void (*done)(); // Ссылка на деструктор единицы объекта
    void **par;    // Список дочерних полей - параметров
    void *cls;   // Базовый класс. Для массивов - просто описатель детей, для структур - базовая структура
    void *extra; // Дополнительные поля - используются для хранения мета-информации в системе хранения
    int  shift; // Используется для ссылки в дочерних полях (смещение хранения поля в списке)
    int  maxSize; // Ограничитель размера (только для Array) - если указан не расширяется более чем
    } objClass;

/*
typedef struct { // Структура типа "блоб"
    int size,len; // Для внешних перераспределений - обратная совместимость с db_blob_handle
    uchar *data;
    int handle;
    } blob;
*/


#define Integer int
#define Number double
//#define Date   double

typedef objClass *pClass;

extern pClass IntegerClass,ByteClass,NumberClass,DateClass,StringClass,BlobClass; // Метка для строк...

typedef struct { // Cтруктура для именованых хешей
    void  *owner; // Владелец ссылки
    uchar *name; // Имя - если ссылка именованная
    void  *data;  // Если ссыла на значение (child)
    } objRef;

typedef struct { // Объектный хедер - перед каждой переменной такой
    void *mem;   // Мемори диспатчер, 0 = статическая переменная
    int   ref;   // Количество ссылок на данный объект (0 - свободен)
    void *proto; // Объект, являющийся прототипом для данного объекта (наследуется из конструктора)
    objRef  **att;   //  void *ch;    //vsHash *ch;  // Дочерние объекты, массив
    // -- поля для обращения к объекту
    //int typ; // Тип объекта (примитивное, массив, ссылка)
    int size1; // Размер единицы хранения в байтах (массивы могут хранить больше чем элемент)
    objClass *cls;  //vsClass *cls; // Класс - ДОЛЖЕН быть определен
    int  size,len; // -- Array -- Выделенный и используемый размер (в элементах = векторах cls)
    } objHead; // 8*4 = 32 байта ---

#define obj2head(obj) ((objHead*)(((char*)obj)-sizeof(objHead)))
#define head2obj(obj) ((void*)(((char*)obj)+sizeof(objHead)))

objHead *objHeadNew(memDisp *md, int size1, int size); // Новый объект+заголовок
int objHeadSize(objHead **h, int newSize); // check - if we have enough size
void *objObjectNew(); // Создание экзхемпляра типа "пустой объект"
char *clsName(void *obj); // Имя класса (для отладки)

void *objAddRef(void *obj); // Gets a new ref to object
objRef *objRefCreate(void *owner) ; // Создаем новую ссылку на объект? добавляем в дети
objRef **attClone(void *owner,objRef **ref) ; // Клонируем аттрибуты
void objClear(void **obj);  // Уменьшает количество ссылок на 1 и удаляет объект если 0

int * IntNew(int val); // Создание нового значения инт

uchar *strNew(uchar *data,int len); // Создание новой строки
uchar *strLoad(uchar *filename); // Загружает строку из файла, NULL при ошибке
int    strSave(uchar *str, uchar *filename); // -1 при ошибке, иначе - размер файла
int strDelete(uchar **str,int pos,int len); // Возвращает - количество удаленных
uchar *strCat(uchar **str, uchar *data, int len); // Добавить в буффер
uchar *strCatFile(uchar **Str,uchar *filename); // Добавляет в строку, загружая ее из файла
uchar *strCatD(uchar **str, uchar *src, int len, int (*decoder)()); // Добавить с декодированием
int    strLength(uchar *str); // Длина строки
inline int    strSetLength(uchar **str, int len); // NewLength

int strEnsureSize(uchar **str,int newLength); // ensures that this string buffer can handle newLength
uchar *strEnsureMore(uchar **str,int addLength); // returns end of current string

inline void strClear(uchar **str);
inline uchar *strPush(uchar ***str, uchar *data);


int arrSetLength(void **arr,int size) ; // Results in length set - it changes arraya anyway!
void *arrayNew(objClass *cls, int sz);
void *arrNew(int size1,int size); // Новый нативный массив
int   arrLength(void *arr); // Длина массива
void *arrAddN(void **arr, int count); // Добавляем в массив, возвращаем начало массива
int   arrDelN(void **arr,int pos,int count) ; // Удаление указанного количества
void *arrAdd(void **arr, void *data); // Добавить в массив
void *arrInsN(void **arr, int pos, int count) ; // Добавляем в массив с позиции


//void *objNew(); // Просто пустой объект
void *objNew(objClass *cls,int size);
void _attClear(objHead *h);
objRef *_attAdd(void *obj); // Стаб рефа для дальнейшего заполнения
objRef *_attAddStr(void *obj,char *name,char *val); // Добавление строкового параметра
void attClear(void *obj); // Удалить всех детей

void **vecNew(int size) ; // Создание массива ссылок на объекты системы
void **_vecPush(void ***vec, void *data);  // Безинкрементное добавление
void ** vecPush(void ***vec, void *data);  // тоже, но с увеличением ссылки
void vecClear(void ***vec); // Удаление вектора (и всех его элементов)
int vecRemove(void ***vec,void *data); // Удаление вектора (и всех его элементов)

/* Генерация класса - конструктора и деструктора по структуре */

typedef struct {
    objHead head;
    objClass cls;
    } StaticClass;

#define VS0OBJ0(fld,doneProc)  \
   StaticClass fld##Head = { .head={.mem=0,.ref=0},\
                             .cls={.typ=0,.name=#fld,.size1=sizeof(fld),.done=doneProc }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline fld* fld##New() { return objNew(&fld##Head.cls,1); }\
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Array(int size) { return arrNew(sizeof(fld),size); };\
   inline fld*   fld##Add(fld **f, fld *data) { \
         if (!*f) *f = (void*) fld##Array(1);\
         return arrAdd((void**)f,data); };\
   inline fld**  fld##Vector(int size) { return (void*)vecNew(size); };\
   inline fld**  fld##Push(fld ***f, fld *data) { return (void*)vecPush((void***)f,(void*)data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}\



#define VS0OBJ00(fld,done)  \
   StaticClass fld##Head = { {0,0},\
                             {0, #fld,sizeof(fld),done }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline fld* fld##New() { return objNew(&fld##Head.cls,1); }\
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Array(int size) { return arrNew(sizeof(fld),size); };\
   inline fld*   fld##Add(fld **f, fld *data) { \
         if (!*f) *f = (void*) fld##Array(1);\
         return arrAdd((void**)f,data); };\
   inline fld**  fld##Vector(int size) { return (void*)vecNew(size); };\
   inline fld**  fld##Push(fld ***f, fld *data) { return (void*)vecPush((void***)f,(void*)data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}\

#define VS0OBJH(fld)  \
   extern objClass *fld##Class;\
   inline fld* fld##New();\
   inline void fld##Clear(fld **f);\
   inline fld*   fld##Array(int size);\
   inline fld*   fld##Add(fld **f, fld *data);\
   inline fld**  fld##Vector(int size);\
   inline fld**  fld##Push(fld ***f, fld *data);\
   inline void   fld##Clean(fld ***f);\

#define VS0OBJ(fld) VS0OBJ0(fld,fld##Done)
#define VS0ARR(fld) VS0OBJ(fld)

/*
//StaticClass fld##Head = { .head={.mem=0,.ref=0},\
  // ZUZUKA                           .cls={.typ=clsArray,.name=#fld,.size1=sizeof(fld),.done=doneProc }};\
*/

#define VS0ARRAY(fld,base1)  \
   StaticClass fld##Head = { .head = {.mem=0,.ref=0},\
                             .cls={.typ=clsArray,.name= #fld,.size1=sizeof(base1),.cls=&base1##Head.cls }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline base1 * fld##New() { return arrayNew(&fld##Head.cls,2); }

#define VS0ARRAY01(fld,base)  \
   StaticClass fld##Head = { .head={.mem=0,.ref=0},\
                             .cls={.typ=clsArray, .name=#fld,.size1=sizeof(void*),.base=&base##Head.cls }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline base * fld##New() { return arrayNew(&fld##Head.cls,2); }


   /*
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Add(fld **f, fld *data) { return arrAdd((void**)f,data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}

   //
*/

int attCount(void *obj); // Количество аттрибутов
int buf2file(uchar *data,int len,uchar *filename); // some utils...




/*
// scaner c - code - )))

#include "vss.h"

typedef struct {
    char **src; // Загруженные файлы -))
    vss  *fun;  // Описания функци -))
    } scan;

#define public
//#define in
//#define out

int scanFile(scan *sc, char *filename);

// Динамические классы

objClass *objClassCreate(char *name, int typ, objClass *base); // Создание нового класса
objClass *objClassAddParam(objClass *cls, objClass *fld); // фтыкание дочерней ссылки внутрь родителя
uchar *objClassDDL(objClass *cls); // Получение DDL по структуре Массив-Хранений
int objClassDump(objClass *cls,int lev);

int *IntNew(int val); // Базовые типы
*/

// child variables -)))

VS0OBJH(Obj)

#endif // VS0
