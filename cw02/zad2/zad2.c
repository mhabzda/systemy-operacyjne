#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

void printStats(struct stat *stat, char *path) {

    char absolutePath[PATH_MAX+1];
    realpath(path, absolutePath);
    printf("File path: %s\n", absolutePath);
    printf("File size: %d bytes\n", (int)stat->st_size);
    printf("File permissions: ");
    if(stat->st_mode & S_IRUSR) printf("r");
    else printf("-");
    if(stat->st_mode & S_IWUSR) printf("w");
    else printf("-");
    if(stat->st_mode & S_IXUSR) printf("x");
    else printf("-");
    if(stat->st_mode & S_IRGRP) printf("r");
    else printf("-");
    if(stat->st_mode & S_IWGRP) printf("w");
    else printf("-");
    if(stat->st_mode & S_IXGRP) printf("x");
    else printf("-");
    if(stat->st_mode & S_IROTH) printf("r");
    else printf("-");
    if(stat->st_mode & S_IWOTH) printf("w");
    else printf("-");
    if(stat->st_mode & S_IXOTH) printf("x");
    else printf("-");
    printf("\n");

    time_t time = stat->st_mtime;
    struct tm *dateTime = localtime(&time);
    char buff[20];

    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", dateTime);
    printf("Last modification: %s\n\n", buff);

}

void search(char *path, int size) {

    DIR *pdir = opendir(path);
    if(pdir == NULL) {
        perror(path);
        exit(1);
    }
    struct dirent *dent;

    while((dent=readdir(pdir)) != NULL) {

        char *fileName = dent->d_name;
        char *newPath;

        if(dent->d_type==DT_DIR) {
            if(strcmp(dent->d_name, "..")!=0 && strcmp(dent->d_name, ".")!=0) {
                newPath = malloc((strlen(path)+strlen(fileName)+1)*sizeof(char));
                newPath[0]='\0';
                strcat(newPath, path);
                strcat(newPath, "/");
                strcat(newPath, fileName);

                search(newPath, size);

                free(newPath);

            }
        }
        else if(dent->d_type==DT_REG){

            newPath = malloc((strlen(path)+strlen(fileName)+1)*sizeof(char));
            newPath[0]='\0';
            strcat(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, fileName);

            struct stat st;
            int n = lstat(newPath, &st);
            if(n==-1) {
                perror(newPath);
            }
            if(st.st_size<=size) printStats(&st, newPath);

            free(newPath);
        }

    }
    closedir(pdir);

}


int main(int argc, char **argv) {

    if(argc!=3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    search(argv[1], atoi(argv[2]));

    return 0;

}

