#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

int fd, op;
char word[32];
pthread_t *tids;
size_t n, nrec;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int sig_num;

void *thread_func(void *arg) {
    if(op == 4) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, sig_num);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
    }

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
            printf("%ld %c%c\n", pthread_self(), record[i][0], record[i][1]);
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
}

int main(int argc, char **argv) {

    if(argc!=6) {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    sig_num = atoi(argv[5]);

    printf("Wpisz\n"
                   "1 - do procesu, brak zamaskowanych sygnalow\n"
                   "2 - do procesu, glowny watek ma zamaskowany sygnal\n"
                   "3 - do procesu, wszystkie watki maja procedure obslugi\n"
                   "4 - do watku z zamaskowanym sygnalem\n"
                   "5 - do watku z obsluga sygnalu\n"
    );

    scanf("%d", &op);

    if(op == 3 || op ==5) {
        struct sigaction act;
        act.sa_handler = signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(sig_num, &act, NULL);
    }
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

    if(op == 2) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, sig_num);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
    }

    tids = calloc(n, sizeof(pthread_t));
    for(int i = 0; i<n; i++) {
        pthread_create(&tids[i], NULL, thread_func, NULL);
    }

    if(op<=3)
        kill(getpid(), sig_num);
    else
        pthread_kill(tids[0], sig_num);

    void *status;
    for(int i=0; i<n; i++) {
        if(pthread_join(tids[i], &status)!=0) {
            printf("Error with pthread_join");
        }
    }

    free(tids);

    return 0;
}


