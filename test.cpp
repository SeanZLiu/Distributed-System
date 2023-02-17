
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h> 




#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>

int main(){
    char *c;
    int fd, sz;

    c = (char *) malloc(11);

    fd = open("/tmp/j254liu/server/myfile.txt", 0x8000);
    if (fd < 0) { perror("r1"); }

    sz = pread(fd, c, 10, 0);

    printf("called read(%d, c, 10).  returned that %d bytes  were read.\n",
            fd, sz);
    c[sz] = '\0';
    printf("Those bytes are as follows: %s\n", c);

    return 0;
}