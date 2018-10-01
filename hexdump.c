int hex_dump(char *msg,uchar *s,int len) {
int i;
 printf("%-30s[%d]\n",msg,len);
 while(len>0) {
 int l=len>16?16:len;
 for(i=0;i<l;i++) printf("%02X ",s[i]);
 for(;i<16;i++) printf("   "); printf(" | ");
 for(i=0;i<l;i++) printf("%c",s[i]>=32?s[i]:'.');
   printf("\n");
   len-=l; s+=l;
  }
return 0;
}


int hexdump(char *msg,uchar *s,int len) {
int i;
 printf("\n%s[%d]:",msg);
 for(i=0;i<len;i++) printf("%02x ",s[i]);
 printf("\n");
return 0;
}
