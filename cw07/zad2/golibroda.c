#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>


int *addr;
sem_t *sem_que, *sem_bar;

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
    munmap(addr, 512);
    shm_unlink("queue");
    sem_close(sem_que);
    sem_close(sem_bar);
}

int main(int argc, char **argv) {

    if(argc != 2) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    int N, shmd;

    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);
    atexit(&release_reserves);

    N = atoi(argv[1]);

    if((shmd = shm_open("/queue", O_RDWR | O_CREAT, 0600))==-1) {
        perror("Error with shm_open");
        return 1;
    }
    if(ftruncate(shmd, 512)==-1) {
        perror("Error with ftruncate");
        return 1;
    }
    if((addr = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, shmd, 0)) == (void*) -1) {
        perror("Error with mmap");
        return 1;
    }

    if((sem_que = sem_open("queue2", O_RDWR | O_CREAT, 0600, 1))==SEM_FAILED) {
        perror("Error with sem_open");
        return 1;
    }
    if((sem_bar = sem_open("barber", O_RDWR | O_CREAT, 0600, 0))==SEM_FAILED) {
        perror("Error with sem_open");
        return 1;
    }

    int b=1, c;
    addr[0]=N;
    addr[1]=0;  //is narber waiting
    addr[2]=0;
    while(b) {
        if(sem_wait(sem_que)==-1) {
            perror("sem_wait");
        }
        if (is_empty(addr + 2)) {
            addr[1]=1;
            if(sem_post(sem_que)==-1) {
                perror("sem_post");
            }
            printf("Czekam");
            get_time();
            if(sem_wait(sem_bar)==-1) {
                perror("sem_wait");
            }
            c = 0;
        }
        else
            c = 1;

        if(!c) {
            if (sem_wait(sem_que) == -1) {
                perror("sem_wait");
            }
        }

        addr[1]=0;
        int client = get(addr+2);

        if(sem_post(sem_que)==-1) {
            perror("sem_post");
        }

        printf("Rozpoczynam strzyzenie klienta nr: %d", client);
        get_time();
        printf("Zakonczylem strzyzenie klienta nr: %d", client);
        get_time();

        kill(client, SIGRTMIN);

        if(c) {
            if(sem_wait(sem_bar)==-1) {
                perror("sem_wait");
            }
        }
    }

}
