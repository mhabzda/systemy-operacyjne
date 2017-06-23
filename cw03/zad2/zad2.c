#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/resource.h>

void addOrRemVar(char *line) {
    char *name = strtok(line, "# \n");
    char *value = strtok(NULL, " \n");

    if(value!=NULL) {
        if(setenv(name, value, 1)==-1) {
            perror("Error with setenv");
            exit(1);
        }
    }
    else {
        if(unsetenv(name)==-1) {
            perror("Error with unsetenv");
            exit(1);
        }
    }
}

void printStats(struct rusage usage) {
    long int user= (usage.ru_utime.tv_sec)*1000 + (usage.ru_utime.tv_usec)/1000;
    long int sys= (usage.ru_stime.tv_sec)*1000 + (usage.ru_stime.tv_usec)/1000;
    printf("User time: %ld ms\n", user);
    printf("System time: %ld ms\n", sys);
    printf("Maximum resident set size used: %ld kb\n", usage.ru_maxrss);
    printf("Block input operations: %ld\n", usage.ru_inblock);
    printf("Block output operations: %ld\n", usage.ru_oublock);
}

void startProcess(char *line, rlim_t time, rlim_t sizeOfMemory) {

    char *args[6];
    int i=0;
    int status;
    struct rusage usage;

    char *token = strtok(line, " \n");
    while(token!=NULL) {
        args[i] = token;
        token = strtok(NULL, " \n");
        i++;
    }
    if(i<6) args[i]=NULL;

    pid_t pid = fork();
    if(pid==0) {
        struct rlimit limit;
        limit.rlim_cur = time;
        limit.rlim_max = time;
        if(setrlimit(RLIMIT_CPU, &limit)==-1) {
            perror("Error with setrlimit");
            exit(1);
        }
        limit.rlim_cur = sizeOfMemory;
        limit.rlim_max = sizeOfMemory;
        if(setrlimit(RLIMIT_AS, &limit)==-1) {
            perror("Error with setrlimit");
            exit(1);
        }

        if(execvp(args[0], args)==-1) {
            perror("Error with execvp");
            exit(1);
        }
    }
    else if(pid>0) {
        wait3(&status, 0, &usage);
        if(WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Potomek \"%s\" zawiodl\n", args[0]);
            exit(1);
        }
        printf("%s:\n", args[0]);
        printStats(usage);
    }
    else {
        perror("Error with fork");
        exit(1);
    }
}


int main(int argc, char **argv) {

    char **line=malloc(sizeof(char*));
    *line=NULL;
    rlim_t time, sizeOfMem;


    if(argc!=4) {
        printf("Bledna liczba argumentow\n");
        exit(1);
    }

    FILE *fd = fopen(argv[1], "r");
    if(fd == NULL) {
        perror(argv[1]);
        exit(1);
    }

    time = (rlim_t) atoi(argv[2]);
    sizeOfMem = (rlim_t) atoi(argv[3]);
    sizeOfMem *= 1024*1024;
    size_t n = 0;
    while(getline(line, &n, fd)!=-1) {

        if(*line[0]=='#') {
            addOrRemVar(*line);
        }
        else {
            startProcess(*line, time, sizeOfMem);
            printf("\n");
        }

        free(*line);
        *line=NULL;

    }

    fclose(fd);
    free(*line);
    free(line);

    return 0;
}





