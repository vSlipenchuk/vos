int bin2hexstr(uchar *out,uchar *bin,int len) {
int i;
if (len<=0) *out=0;
for(i=0;i<len;i++) sprintf(out+2*i,"%02X",bin[i]);
return 2*len;
}

