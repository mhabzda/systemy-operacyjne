#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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

void startProcess(char *line) {

    char *args[6];
    int i=0;
    int status;

    char *token = strtok(line, " \n");
    while(token!=NULL) {
        args[i] = token;
        token = strtok(NULL, " \n");
        i++;
    }
    if(i<6) args[i]=NULL;

    pid_t pid = fork();
    if(pid==0) {
        if(execvp(args[0], args)==-1) {
            perror("Error with execvp");
            exit(1);
        }
    }
    else if(pid>0) {
        wait(&status);
        if(WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Potomek \"%s\" zawiodl\n", args[0]);
            exit(1);
        }
    }
    else {
        perror("Error with fork");
        exit(1);
    }
}


int main(int argc, char **argv) {

    char **line=malloc(sizeof(char*));
    *line=NULL;

    if(argc!=2) {
        printf("Bledna liczba argumentow\n");
        exit(1);
    }

    FILE *fd = fopen(argv[1], "r");
    if(fd == NULL) {
        perror(argv[1]);
        exit(1);
    }

    size_t n = 0;
    while(getline(line, &n, fd)!=-1) {

        if(*line[0]=='#') {
            addOrRemVar(*line);
        }
        else {
            startProcess(*line);
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



