#include <stdio.h>
#include <stdlib.h>

#include "vos.c"
#include "exe.c"
#include "vs0.c"
#include "logger.c"
#include "coders.c"
#include "httpTest.c"
#include "httpSrv.c"
#include "sock.c"
#include "vss.c"
#include "std_sock.c"


int main(int npar,char **par)
{
    return MicroHttpMain(npar,par);
}
