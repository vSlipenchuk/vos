int hex_dump(char *msg,uchar *s,int len) {
int i;
 printf("\n%s:",msg);
 for(i=0;i<len;i++) printf("%02x ",s[i]);
 printf("\n");
return 0;
}


int hexdump(char *msg,uchar *s,int len) {
int i;
 printf("\n%s:",msg);
 for(i=0;i<len;i++) printf("%02x ",s[i]);
 printf("\n");
return 0;
}
