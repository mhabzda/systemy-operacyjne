#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>
#include "golibroda.h"

int *addr, shmid, semid;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

int is_empty(int* queue) {
    if(queue[0]==0) return 1;
    else return 0;
}

int get(int *queue) {
    if(!is_empty(queue)) {
        int elem = queue[1];
        int pos = queue[0];
        queue[0]--;
        for (int i = 1; i < pos ; i++) {
            queue[i] = queue[i + 1];
        }
        return elem;
    }
    else {
        return -1;
    }
}

void get_time() {
    struct timespec tp;
    if(clock_gettime(CLOCK_MONOTONIC, &tp)==-1) {
        perror("clock_gettime");
    }
    printf(" %ld\n", tp.tv_sec * 1000000 + tp.tv_nsec/1000);
}

void handler(int signum) {
    if(signum==SIGINT) {
        exit(0);
    }
}

void release_reserves(void) {
    shmdt(addr);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID, NULL);
}

int main(int argc, char **argv) {

    if(argc != 2) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    int N;
    char *homedir;

    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);
    atexit(&release_reserves);

    N = atoi(argv[1]);
    homedir = getenv("HOME");

    key_t key = ftok(homedir, PROJ);
    if(key == -1) {
        perror("Error with ftok");
        return 1;
    }
    if((shmid = shmget(key, 512, IPC_CREAT | 0600))==-1) {
        perror("Error with shmget");
        return 1;
    }
    if((addr = shmat(shmid, NULL, 0)) == (void*) -1) {
        perror("Error with shmat");
        return 1;
    }

    key = ftok(homedir, PROJ2);
    if(key == -1) {
        perror("Error with ftok");
        return 1;
    }
    if((semid = semget(key, 2, IPC_CREAT | 0600))==-1) {
        perror("Error with semget");
        return 1;
    }
    union semun arg;
    arg.val=1;
    if(semctl(semid, 0, SETVAL, arg)==-1) {
        perror("Error with semctl");
        return 1;
    }
    arg.val=0;
    if(semctl(semid, 1, SETVAL, arg)==-1) {
        perror("Error with semctl");
        return 1;
    }

    int b=1, c;
    struct sembuf sops1, sops2;
    addr[0]=N;
    addr[1]=0;  //is burber waiting
    addr[2]=0;
    sops1.sem_num = 0;
    sops1.sem_flg = SEM_UNDO;
    sops2.sem_num = 1;
    sops2.sem_flg = SEM_UNDO;
    while(b) {
        sops1.sem_op=-1;
        if(semop(semid, &sops1, 1)==-1) {
            perror("semop");
        }
        if (is_empty(addr + 2)) {
            addr[1] = 1;
            sops1.sem_op=1;
            if(semop(semid, &sops1, 1)==-1) {
                perror("semop");
            }
            printf("Czekam");
            get_time();
            sops2.sem_op=-1;
            if(semop(semid, &sops2, 1)==-1) {
                perror("semop");
            }
            c = 0;
        }
        else
            c = 1;

        if(!c) {
            sops1.sem_op = -1;
            if (semop(semid, &sops1, 1) == -1) {
                perror("semop");
            }
        }

        addr[1] = 0;
        int client = get(addr+2);

        sops1.sem_op=1;
        if(semop(semid, &sops1, 1)==-1) {
            perror("semop");
        }

        printf("Rozpoczynam strzyzenie klienta nr: %d", client);
        get_time();
        printf("Zakonczylem strzyzenie klienta nr: %d", client);
        get_time();

        kill(client, SIGRTMIN);

        if(c) {
            sops2.sem_op=-1;
            if(semop(semid, &sops2, 1)==-1) {
                perror("semop");
            }
        }
    }

}