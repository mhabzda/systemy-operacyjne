#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include "golibroda.h"

int semid, waiting;

void get_time() {
    struct timespec tp;
    if(clock_gettime(CLOCK_MONOTONIC, &tp)==-1) {
        perror("clock_gettime");
    }
    printf(" %ld\n", tp.tv_sec * 1000000 + tp.tv_nsec/1000);
}

int is_empty(int* queue) {
    if (queue[0] == 0) return 1;
    else return 0;
}

int is_full(int *queue, int N) {
    if(queue[0]==N) {
        return 1;
    }
    else {
        return 0;
    }
}

void put(int *queue, int elem) {
    queue[0]++;
    int pos = queue[0];
    queue[pos] = elem;
}

void do_action(int *queue, int id, char* announ) {
    put(queue, id);
    printf("%d: %s", id, announ);
    get_time();
    struct sembuf sops;
    sops.sem_num = 1;
    sops.sem_flg = 0;
    sops.sem_op = 1;
    if(semop(semid, &sops, 1)==-1) {
        perror("semop");
    }
    waiting = 1;
}

void handler(int signum) {
    waiting = 0;
}

int main(int argc, char **argv) {

    if(argc!=3) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGRTMIN, &act, NULL);

    char *homedir;
    int K, S, shmid;
    pid_t pid;
    K = atoi(argv[1]);
    S = atoi(argv[2]);
    char *wake_up = "Obudzilem golibrode";
    char *take_seat = "Zajalem miejsce w poczekalni";
    waiting = 0;

    homedir = getenv("HOME");
    key_t key = ftok(homedir, PROJ);
    if(key == -1) {
        perror("Error with ftok");
        return 1;
    }
    if((shmid = shmget(key, 0, 0))==-1) {
        perror("Error with shmget");
        return 1;
    }

    key = ftok(homedir, PROJ2);
    if(key == -1) {
        perror("Error with ftok");
        return 1;
    }
    if((semid = semget(key, 0, 0))==-1) {
        perror("Error with semget");
        return 1;
    }
    for(int i=0; i<K; i++) {
        pid = fork();
        if(pid==0) {
            int *addr, id;
            if((addr = shmat(shmid, NULL, 0)) == (void*) -1) {
                perror("Error with shmat");
                return 1;
            }
            id = getpid();

            int b=1;

            struct sembuf sops;
            sops.sem_num=0;
            sops.sem_flg=SEM_UNDO;
            for(int j=0; j<S && b; j++) {
                sops.sem_op=-1;
                if(semop(semid, &sops, 1)==-1) {
                    perror("semop");
                }
                if(is_empty(addr+2) && addr[1]) {
                    do_action(addr+2, id, wake_up);
                }
                else if(!is_full(addr+2, addr[0])) {
                    do_action(addr+2, id, take_seat);
                }
                else {
                    printf("%d: Brak wolnych miejsc- opuszczam zaklad", id);
                    get_time();
                    b=0;
                }
                sops.sem_op = 1;
                if(semop(semid, &sops, 1)==-1) {
                    perror("semop");
                }

                while(waiting);
            }
            if(b) {
                printf("%d: Zakonczylem strzyzenie- opuszczam zaklad", id);
                get_time();
            }

            shmdt(addr);
            exit(0);
        }
        else if(pid == -1) {
            perror("Error with fork");
            return 1;
        }
    }

    for(int i = 0; i < K; i++){
        int status;
        wait(&status);
    }
    return 0;
}

