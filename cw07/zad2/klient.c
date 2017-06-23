#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>


int semid, waiting;
sem_t *sem_que, *sem_bar;

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
    if(sem_post(sem_bar)==-1) {
        perror("sem_post");
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

    int K, S, shmd;

    pid_t pid;
    K = atoi(argv[1]);
    S = atoi(argv[2]);
    char *wake_up = "Obudzilem golibrode";
    char *take_seat = "Zajalem miejsce w poczekalni";


    if((shmd = shm_open("/queue", O_RDWR, 0))==-1) {
        perror("Error with shm_open");
        return 1;
    }

    if((sem_que = sem_open("queue2", O_RDWR))==SEM_FAILED) {
        perror("Error with sem_open");
        return 1;
    }
    if((sem_bar = sem_open("barber", O_RDWR))==SEM_FAILED) {
        perror("Error with sem_open");
        return 1;
    }
    for(int i=0; i<K; i++) {
        pid = fork();
        if(pid==0) {
            int *addr, id;
            if((addr = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, shmd, 0)) == (void*) -1) {
                perror("Error with mmap");
                return 1;
            }
            id = getpid();

            int b=1;

            for(int j=0; j<S && b; j++) {
                if(sem_wait(sem_que)==-1) {
                    perror("sem_wait");
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
                if(sem_post(sem_que)==-1) {
                    perror("sem_post");
                }

                while(waiting);
            }
            if(b) {
                printf("%d: Zakonczylem strzyzenie- opuszczam zaklad", id);
                get_time();
            }

            munmap(addr, 512);
            return 0;
        }
        else if(pid == -1) {
            perror("Error with fork");
            return 1;
        }
    }

    for(int i=0; i<K; i++) {
        int status;
        wait(&status);
    }
    return 0;
}



