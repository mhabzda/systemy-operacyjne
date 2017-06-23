#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int writers, readers;
int data[30];

const int TURNS = 30;
int d;

void *writer(void *arg) {

    int pos[5], val[5];
    for(int i=0; i<TURNS; i++) {

        pthread_mutex_lock(&mutex);
        while(readers!=0 || writers!=0) {
            pthread_cond_wait(&cond, &mutex);
        }
        writers++;
        pthread_mutex_unlock(&mutex);

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

        pthread_mutex_lock(&mutex);
        writers--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *reader(void *arg) {

    for (int i = 0; i < TURNS; i++) {

        pthread_mutex_lock(&mutex);
        while(writers==1) {
            pthread_cond_wait(&cond, &mutex);
        }
        readers++;
        pthread_mutex_unlock(&mutex);

        printf("%ld: Szukam liczb mod%d\n", pthread_self(), d);
        for (int j = 0; j < 30; j++) {
            if (data[j] % d == 0 && data[j]!=0) {
                printf("%ld: Znalazlem liczbe %d na pozycji %d\n", pthread_self(), data[j], j);
            }
        }

        pthread_mutex_lock(&mutex);
        readers--;
        if(readers==0) {
            pthread_cond_broadcast(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {

    d = atoi(argv[1]);
    srand((unsigned) time(NULL));

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

