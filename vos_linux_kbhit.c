#include "vos.h"
#ifdef __linux__

#include <termios.h>
#include <stropts.h>
#include <stdio.h>
#include <sys/ioctl.h>

void kbecho(int echo) {
static const int STDIN = 0;
 struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~(ICANON );
        if (!echo) term.c_lflag &= ~(ECHO );
             else term.c_lflag |= ECHO;

       // term.
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);

}

int kbhit() {
    static const int STDIN = 0;
    static int initialized = 0;

    if (! initialized) {
         kbecho(0);
         /*
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~(ICANON );
        term.c_lflag &= ~(ECHO );

       // term.
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        */
        initialized = 1;

    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}


#endif
