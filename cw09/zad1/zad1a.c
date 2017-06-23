#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    int head;
    int tail;
} fifo_t;

sem_t mutex, writer_sem;
int readers;
int data[30];
fifo_t fifo;

const int TURNS = 30;
int d;

void *writer(void *arg) {

    int pos[5], val[5];
    for(int i=0; i<TURNS; i++) {

        sem_wait(&mutex);
        int k = fifo.tail;
        fifo.tail++;
        sem_post(&mutex);
        while(k!=fifo.head);

        sem_wait(&writer_sem);

        sem_wait(&mutex);
        fifo.head++;
        sem_post(&mutex);

        int n = rand()%5+1;
        for(int j=0; j<n; j++) {
            pos[j] = rand()%30;
            val[j] = rand()%50+1;
        }
        printf("%ld: Wylosowalem %d liczb(y)\n", pthread_self(), n);
        for(int j=0; j<n; j++) {
            printf("%ld: Wpisuje %d na pozycje %d\n", pthread_self(), val[j], pos[j]);
            data[pos[j]] = val[j];
        }

        sem_post(&writer_sem);
    }

    return NULL;
}

void *reader(void *arg) {

    for(int i=0; i<TURNS; i++) {

        sem_wait(&mutex);
        int k = fifo.tail;
        fifo.tail++;
        sem_post(&mutex);
        while(k!=fifo.head);

        sem_wait(&mutex);
        readers++;
        if(readers == 1)
            sem_wait(&writer_sem);
        fifo.head++;
        sem_post(&mutex);

        printf("%ld: Szukam liczb mod%d\n", pthread_self(), d);
        for(int j=0; j<30; j++) {
            if(data[j] % d == 0 && data[j]!=0) {
                printf("%ld: Znalazlem liczbe %d na pozycji %d\n", pthread_self(), data[j], j);
            }
        }

        sem_wait(&(mutex));
        readers--;
        if(readers==0)
            sem_post(&writer_sem);
        sem_post(&mutex);

    }
    return NULL;
}

int main(int argc, char **argv) {

    d = atoi(argv[1]);
    srand((unsigned) time(NULL));

    sem_init(&mutex, 0, 1);
    sem_init(&writer_sem, 0, 1);
    readers = 0;
    fifo.head=0;
    fifo.tail=0;

    pthread_t *tids = calloc(6, sizeof(pthread_t));
    int i = 0;
    for(; i<1; i++) {
        pthread_create(&tids[i], NULL, &writer, NULL);
    }

    for(; i<3; i++) {
        pthread_create(&tids[i], NULL, &reader, NULL);
    }

    for(; i<4; i++) {
        pthread_create(&tids[i], NULL, &writer, NULL);
    }

    for(; i<6; i++) {
        pthread_create(&tids[i], NULL, &reader, NULL);
    }

    void *status;
    for(int j=0; j<6; j++) {
        if(pthread_join(tids[j], &status)!=0) {
            printf("Error with pthread_join");
        }
    }
    free(tids);

    return 0;
}

