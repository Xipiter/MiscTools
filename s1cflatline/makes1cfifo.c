#include "s1cflatline.h"

int main () {
	unsigned short newmode;
	int mk;
        newmode = 0666;
        mk = mkfifo (FIFOFILE, newmode);
        if (mk != 0) { perror("MkFIFO failed"); exit(1);};
        //chmod(FIFOFILE, S_IWOTH);
	return 0;
}
