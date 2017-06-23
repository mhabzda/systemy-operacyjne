#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void) {

    char **line=malloc(sizeof(char*));
    *line=NULL;
    size_t n = 0;

    int b, j, commands;
    char *args[5];
    int fd[20][2];

    while(getline(line, &n, stdin)!=1) {

        commands=0;
        char *token = strtok(*line, " \n");
        args[0] = token;

        for (int i = 0; i < 20 && token != NULL; i++) {

            b = 0;
            if (i != 0) j = 0;
            else j=1;

            while (token != NULL && !b) {
                token = strtok(NULL, " \n");
                if (token == NULL || strcmp(token, "|") == 0) {
                    b = 1;
                    commands++;
                }
                if(!b) {
                    args[j] = token;
                    j++;
                }

            }
            args[j] = NULL;

            pipe(fd[i]);

            pid_t pid = fork();
            if (pid == 0) {

                close(fd[i][0]);
                if (i != 0) close(fd[i-1][1]);
                if (token) {
                    if (dup2(fd[i][1], STDOUT_FILENO) != STDOUT_FILENO) {
                        perror("Error with dup2");
                        exit(1);
                    }
                    close(fd[i][1]);
                }
                if (i != 0) {
                    if (dup2(fd[i-1][0], STDIN_FILENO) != STDIN_FILENO) {
                        perror("Error with dup2");
                        exit(1);
                    }
                    close(fd[i-1][0]);
                }
                execvp(args[0], args);
            } else if (pid < 0) {
                perror("Error with fork");
                exit(1);
            }
            if(i!=0) {
                close(fd[i-1][0]);
                close(fd[i-1][1]);
            }
        }

        free(*line);
        *line = NULL;
        for (int i = 0; i < commands; i++) {
            int status;
            if (wait(&status) == -1) {
                perror("Error with wait");
                return 1;
            }
        }
    }
    free(line);

    return 0;
}

