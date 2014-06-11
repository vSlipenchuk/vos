#ifndef CODERS_H
#define CODERS_H

#include "vtypes.h"
// strutil.c

uchar *get_row(uchar **tbl); //@strutil.c
uchar *get_col(uchar **row); //@strutil.c
int get_cols(uchar **row,uchar *fmt,...); //@strutil.c
uchar *get_word(uchar **str); //@strutil.c gets a word or ""
int get_int(uchar **str); //@strutil.c
int lcmp(uchar **str,uchar *cmp); //@strutil.c
int strnstr(uchar *str,int sl,uchar *del,int dl); // Поиск подстроки
uchar *trim(uchar *src); // left+right trim
uchar *ltrim(uchar *src); // left trim
uchar *get_till(uchar **data,uchar *del,int dl); // split a string...
/*

 Декларации разнообразных декодеров

*/


int gsm7_decode(uchar *dst, uchar *src, int len, int offset);

int md5sign(uchar *d,uchar *s,int l); //@md5.c  Sign this MD5 message
int md5hash(uchar *d,uchar *s,int l); //@md5.c Signs & show it ans hex...
int md5hash_(uchar *dst,uchar *src,int len,...); //@md5

int c_decode(uchar *d,uchar *s,int l);
int c_encode(uchar *d,uchar *s,int l);
int q_encode(char *dst, char *src, int len) ; // double print ''

int encode_base64(unsigned char *d,unsigned char *s,int slen);
int decode_base64(unsigned char *d,unsigned char *s,int slen);

int encode_uu(uchar *d, uchar *s, int len);

int encode_utf8(uchar *d,uchar *s,int sl); // Results in UTF8 ...
int decode_utf8(uchar *d,uchar *s,int sl); // Results in UTF8 ...

extern unsigned char win_dos[256];
extern unsigned char dos_win[256];

extern u_short win1251_to_unicode[256]; //u_short win1251_to_unicode[256]={

extern unsigned char koi_win[256];
extern unsigned char win_koi[256];
extern unsigned char win_dos[256];
extern unsigned char dos_win[256];

uchar *CP_DECODE(uchar *dst, uchar *src, int len, uchar *tbl);
#define dos2win(A,B,LEN) CP_DECODE(A,B,LEN,dos_win)
#define win2dos(A,B,LEN) CP_DECODE(A,B,LEN,win_dos)

char *cp_decode(unsigned char *src, unsigned char *cp);

int hex(uchar src); // -1 on invalid

int win2gsm(uchar *d,uchar *src,int len);
int unicode_to_str(uchar *buf,int size,uchar *wbuf,int wsize);

uchar unichar(uchar page, uchar code, uchar def); // win2151 <- unichar
int hexstr2bin(uchar *out,uchar *in, int len); // converts 00ffbbcc -> len/2

int gsm2win(uchar *dst,uchar *src,int len); // gsm unicode converter


// ----- GSM coders

enum { // перечисление для mt...
  SMS_DELIVER = 0, // dir=in, Сообщение принимается из сети на телефон
  SMS_SUBMIT = 1, //  dir=out, Сообщение отсылается с телефона в сеть
  SMS_REPORT = 2, //  Нотификация о доставке отправленного сообщения (или SMS_COMMAND при отправке в сеть)
  SMS_RESERVED = 3
  };

enum { // DCS Indicators SMS
  smsInd = 0xC0,
  indVms = 0, //v
  indFax = 1, //f
  indEmail = 2, //e
  indOther = 3, //o
  indOn = 8, //+
  };

enum { // SMS_STATUS (on TP-STATUS) 9.2.3.15 (OK=<0x20 .. TMP ... FAIL>=0x40)
// === Short message transaction completed, OK
   sms_received = 0000000,  //Short message received by the SME
   sms_forwarded = 0000001, // Short message forwarded by the SC to the SME but the SC is unable to confirm delivery
   sms_replaced =  2, //0000010 Short message replaced by the SC
   //Reserved values
   //0000011..0001111 Reserved
   //0010000..0011111 Values specific to each SC
// === ---Temporary error, SC still trying to transfer SM
   sms_temp_congestion = 0x20, // 0b0100000 , // Congestion
   sms_temp_busy = 0x21, ///*0100001 SME busy
   sms_temp_noresp = 0x022, //0100010 No response from SME
   sms_temp_reject = 0x023, //0100011 Service rejected
   sms_temp_qos = 0x24, //0100100 Quality of service not available
   sms_temp_sme_err = 0x25 , //0100101 Error in SME
   // mts responds with code=48=0x30 when buffer a message for delivery -))))
   /*
   0100110..0101111 Reserved
   0110000..0111111 Values specific to each SC
   */
//  ==== Permanent error, SC is not making any more transfer attempts
    sms_rpc_err = 0x40, //    1000000 Remote procedure error
    sms_dest_err = 0x41, //1000001 Incompatible destination
    sms_rej_err =  0x42, //1000010 Connection rejected by SME
    sms_obtain_err = 0x43, //1000011 Not obtainable
    sms_qos_err = 0x44, //01000100 Quality of service not available
    sms_interwork_err = 0x45, //1000101 No interworking available
    sms_expired = 0x46, // 1000110 SM Validity Period Expired
    sms_deleted_sme = 0x47, // 1000111 SM Deleted by originating SME
    sms_deleted_admin = 0x48, //1001000 SM Deleted by SC Administration,
    sms_nodest = 0x49, //1001001 SM does not exist (The SM may have previously existed in  the SC but the SC no longer has knowledge of it or the SM
                       // may never have previously existed in the SC)
/*
1001010..1001111 Reserved
1010000..1011111 Values specific to each SC
*/
// -- Temporary error, SC is not making any more transfer attempts
    sms_congestion = 0x60, //1100000 Congestion
    sms_busy = 0x61, //1100001 SME busy
    sms_noresponse = 0x62, // 1100010 No response from SME
    sms_reject = 0x63, //1100011 Service rejected
    sms_qos_ = 0x64, //1100100 Quality of service not available
    sms_sme_err = 0x65, // 1100101 Error in SME
 /*
 1100110..1101001 Reserved
 1101010..1101111 Reserved
 1110000..1111111 Values specific to each SC
 */

 };


typedef struct {  // by GSM 03.40
  int mti;   // MessageTypeIndicator PDU, 2 bit
  int rd;   // RejectDuplicates, 1 bit
  int vpf;  // SUBMIT ValidityPeriodFormat, 2 bit
  int mms;  // DELIVER - MoreMessagesToSend - 1 bit
  int rp;   // ReplayPath, 1 bit
  int udhi; // UserDataHeader, 1 bit
  int sr;  // StatusReport (Request for SUMBIT or indicator for DELIVR ), 1 bit
            // --- end of first octet ------
  int  mr; // !!! DELIVER - MessageReference
  char da[28]; // Destination Address (up to 12 octet in source stream) or src addr for DELIVR or report...
  int  pid; // Protocol Identifier, - 1 octet
  int  dcs; // DataCodingCheme  - 1 octet
  char vp[16]; // ValidityPeriod, up to 7 octet in stream (YYMMDDHHNNSS+TZ)
  int  udl; // UserDataLength - 1 octet
  unsigned char *ud; // UserData - Depend of udhi & cs !!!
  unsigned char *udh; // MayBy NULL if no UDHI...
  char dtime[16]; // STATUS - время дисчаржда ...
  int  status; // STATUS - Статус сообщения
  unsigned char text[512]; // Сообщение в Win1251
  unsigned char error[256]; // Ошибка декодирования ...
  uchar payload[256]; int payload_length; // copy source payload (when decoding)

  unsigned char data[256]; // Бинарное значение кодируемого-декодируемого СМС
  int chains, total, segment; // Это все информация о сегменте (транзакция, всего сегментов, текущий сегмент
  int offset,smHead,smLen,payload_segment,sm_udhl; unsigned char *slen; // Смещение, для кодирования GSM7bit, указатель - где лежит длина, и данные
  char *szText; int Len; // Указатель на входящий поток для кодирования
  // Специальные поля для SMS ME - работы с сообщениями в телефоне
  char smsc[128]; // Адрес сервис-центра, начинает сообщение в стриме полученном от телефона ..

  uchar hex[512]; // Source in a hex data (or empty) - как тут быть - это С указанным SMSC или без??? - для телефона - С
  int pos; // Указатель внутри телефона для данной СМС???

  } t_sms; // Мое понимание СМС...


// Декодирует сообщение из потока (>0) или возвращает ошибку ...
int sms_decode(t_sms *s, unsigned char *src, int len);
int sms_encode_ex(unsigned char *s, char *addr,
 int msgRef,int reqReport, int isFlash, unsigned char *text);
 int sms_dump(unsigned char *s,int len, int smsc); // Before decode???
 int sms_SI(char *sms, char *phone, char *url, char *title);
 int wap_encode_si(unsigned char *d, char *href, char *title);
 int sms_encode_ind(char *sms,char *phone,char *D); // indicators
int sms_decode2(t_sms *sm, unsigned char *s, int len);
int udh_wapPorts(uchar *UDHI,unsigned short dest,unsigned short src);

int sms_get_addr2(unsigned char *addr,unsigned char *src,int len);
int sms_oct(unsigned char *dst,unsigned char *src,int len) ; // Вытаскивает октеты ...
int prn_sms(t_sms *sm);
int sms_put_addr2(char *dst, char *addr);

// Дебуг
int hex_dump(char *msg,uchar *s,int len); // где лежит???

// some utf8 coders

int utf2gsm(uchar *d,uchar *src,int len); // code to GSM UCS2
int utf_nonstd(uchar *src,int len); // calc non-eng letters
int utf2koi(uchar *dst, uchar *src, int len); // utf ->UNI->win->koi

#endif // CODERS_H
