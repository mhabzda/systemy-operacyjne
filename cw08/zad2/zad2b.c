#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

int fd;
char word[32];
pthread_t *tids;
size_t n, nrec;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *thread_func(void *arg) {

    char *id = calloc(2, sizeof(char));
    char **record = calloc(nrec, sizeof(char *));
    for(int i = 0; i<nrec; i++) {
        record[i] = calloc(1024, sizeof(char));
    }
    ssize_t bytes_read = 0;

    do {
        pthread_mutex_lock(&mtx);

        for (int i = 0; i < nrec; i++) {
            if ((bytes_read = read(fd, record[i], 1024)) == -1) {
                perror("read");
            }
        }

        pthread_mutex_unlock(&mtx);

        for (int i = 0; i < nrec && bytes_read; i++) {
            if (strstr(record[i], word) != NULL) {
                strncpy(id, record[i], 2);
                printf("%ld found word in record: %s\n", pthread_self(), id);
                bytes_read = 0;
            }
            //printf("%ld %s\n", pthread_self(), record[i]);
        }
    } while(bytes_read);

    free(id);
    for(int i=0; i<nrec; i++) {
        free(record[i]);
    }
    free(record);

    return NULL;
}

void signal_handler(int signum) {
    printf("Caught signal %d, PID: %d, TID: %ld\n", signum, getpid(), pthread_self());
//    kill(getpid(), SIGKILL);
}

int main(int argc, char **argv) {

    if(argc!=5) {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    struct sigaction act;
    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGFPE, &act, NULL);
    printf("Main thread: %ld\n", pthread_self());

    n = (size_t) atoi(argv[1]);
    char name[32];
    nrec =(size_t) atoi(argv[3]);
    strcpy(name, argv[2]);
    strcpy(word, argv[4]);

    if((fd = open(name, O_RDONLY))==-1) {
        perror("open");
        return 1;
    }

    tids = calloc(n, sizeof(pthread_t));
    for(int i = 0; i<n; i++) {
        pthread_create(&tids[i], NULL, thread_func, NULL);
    }

    int x = 10/0;

    void *status;
    for(int i=0; i<n; i++) {
        if(pthread_join(tids[i], &status)!=0) {
            printf("Error with pthread_join");
        }
    }

    free(tids);

    return 0;
}


