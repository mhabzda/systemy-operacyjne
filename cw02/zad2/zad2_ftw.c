#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <ftw.h>

int SIZE;

int ftwFun(const char *fpath, const struct stat *statptr, int tflag) {

    if(tflag == FTW_F && statptr->st_size<=SIZE) {
        char absolutePath[PATH_MAX + 1];
        realpath(fpath, absolutePath);
        printf("File path: %s\n", absolutePath);

        printf("File size: %d bytes\n", (int) statptr->st_size);
        printf("File permissions: ");
        if (statptr->st_mode & S_IRUSR) printf("r");
        else printf("-");
        if (statptr->st_mode & S_IWUSR) printf("w");
        else printf("-");
        if (statptr->st_mode & S_IXUSR) printf("x");
        else printf("-");
        if (statptr->st_mode & S_IRGRP) printf("r");
        else printf("-");
        if (statptr->st_mode & S_IWGRP) printf("w");
        else printf("-");
        if (statptr->st_mode & S_IXGRP) printf("x");
        else printf("-");
        if (statptr->st_mode & S_IROTH) printf("r");
        else printf("-");
        if (statptr->st_mode & S_IWOTH) printf("w");
        else printf("-");
        if (statptr->st_mode & S_IXOTH) printf("x");
        else printf("-");
        printf("\n");

        time_t time = statptr->st_mtime;
        struct tm *dateTime = localtime(&time);
        char buff[20];

        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", dateTime);
        printf("Last modification: %s\n\n", buff);
    }
    return 0;

}

int main(int argc, char **argv) {

    if(argc!=3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }


    SIZE = atoi(argv[2]);
    if(ftw(argv[1], ftwFun, 1)==-1) {
        printf("Error with ftw. Maybe wrong directory.\n");
    }

    return 0;

}

