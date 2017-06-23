#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct node {
    int n;
    int type; //1-read, 2-write
    struct node *next;
} Node;
Node *first=NULL;

void addToList(int n, int type) {
    Node *p = malloc(sizeof(*p));
    p->n=n;
    p->type=type;

    p->next=first;
    first=p;
}

void removeFromList(int n) {
    Node *p=first;
    Node *q=NULL;
    while(p!=NULL) {
        if(p->n==n) {
            if (p == first) {
                first = p->next;
                p->next = NULL;
            } else {
                q->next = p->next;
                p->next = NULL;
            }
            free(p);
            p=NULL;
        }
        q=p;
        if(p!=NULL) p=p->next;
    }
}

int checkLocks(int n) {
    Node *p=first;
    while(p!=NULL) {
        if(p->n==n) {
            if(p->type==1) return 1;
            else return 2;
        }
        p=p->next;
    }
    return -1;
}

int setLock(int fd, int n, int option) {

    if(checkLocks(n)==1 && option==1) return 1;
    if(checkLocks(n)==2 && option==2) return 1;
    if(checkLocks(n)==2 && option==1) return 1;

    struct flock fl;
    char opt[11];

    if(option==1) {
        fl.l_type = F_RDLCK;
        strcpy(opt, "do odczytu");
    }
    else {
        fl.l_type = F_WRLCK;
        strcpy(opt, "do zapisu");
    }
    fl.l_whence = SEEK_SET;
    fl.l_start = n;
    fl.l_len = 1;

    if(fcntl(fd, F_SETLK, &fl)==-1) {
        if(errno == EACCES || errno == EAGAIN) {
            printf("Bajt nr %d zaryglowany przez inny proces\n", n);
        }
        else {
            printf("Blad z fcntl w setLock\n");
        }
        return -1;
    }
    else {
        printf("Rygiel %s zalozony na bajcie numer: %d\n", opt, (int)fl.l_start);
        if(option==1) addToList(n, 1);
        else addToList(n, 2);
        return 1;
    }
}

void setLock_block(int fd, int n, int option) {

    if(checkLocks(n)==1 && option==1) return;
    if(checkLocks(n)==2 && option==2) return;

    struct flock fl;
    char opt[11];

    if(option==1) {
        fl.l_type = F_RDLCK;
        strcpy(opt, "do odczytu");
    }
    else {
        fl.l_type = F_WRLCK;
        strcpy(opt, "do zapisu");
    }
    fl.l_whence = SEEK_SET;
    fl.l_start = n;
    fl.l_len = 1;

    if(fcntl(fd, F_SETLKW, &fl)==-1) {
        printf("Blad z fcntl w setLock_W\n");
    }
    else {
        printf("Rygiel %s zalozony na bajcie numer: %d\n", opt, (int)fl.l_start);
        if(option==1) addToList(n, 1);
        else addToList(n, 2);
    }
}

void unlock(int fd, int n) {

    if(checkLocks(n)==-1) {
        printf("Proba zdjecia rygla z niezaryglowanego bajtu.\n");
        return;
    }

    struct flock fl;

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = n;
    fl.l_len = 1;

    if(fcntl(fd, F_SETLK, &fl)==-1) {
        printf("Blad z fcntl w unlock\n");
    }
    else {
        printf("Rygiel odblokowany na bajcie numer: %d\n", (int)fl.l_start);
        removeFromList(n);
    }
}

long getLength(int fd) {

    long bytes = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return bytes;

}

void readByte(int fd, int n) {

    long bytes = getLength(fd);
    if(n>=bytes){
        printf("Podano za duzy indeks");
        return;
    }

    int ul = checkLocks(n);

    if(setLock(fd, n, 1)==1) {
        unsigned char byte='\0';
        lseek(fd, n, SEEK_SET);
        read(fd, &byte, 1);
        printf("Odczytano znak: 0x%01x\n", byte);
        if(ul==-1) unlock(fd, n);
    }
    else
        printf("Nie mozna odczytac znaku\n");



}

void writeByte(int fd, int n, unsigned char c) {

    long bytes = getLength(fd);
    if(n>bytes){
        printf("Podano za duzy indeks\n");
        return;
    }

    int ul = checkLocks(n);

    if(setLock(fd, n, 2)==1) {
        lseek(fd, n, SEEK_SET);
        write(fd, &c, 1);
        printf("Wpisano znak: 0x%01x\n", c);
        if(ul==-1 || ul==1) unlock(fd, n);
    }
    else
        printf("Nie mozna zamienic znaku\n");
}

void list(int fd) {

    long bytes = getLength(fd);
    struct flock fl;

    printf("Moje rygle:\n");
    Node *p=first;
    while(p!=NULL) {
        char type[10];
        if(p->type==1)
            strcpy(type, "Odczyt");
        else
            strcpy(type, "Zapis");
        unsigned char byte='\0';
        lseek(fd, p->n, SEEK_SET);
        read(fd, &byte, 1);
        printf("Zaryglowany znak 0x%01x nr %d\n", byte, p->n);
        printf("PID: %d\nTyp: %s\n\n", getpid(), type);
        p=p->next;
    }

    printf("Rygle innych procesow:\n");
    for(int i=0; i<bytes; i++) {

        fl.l_type=F_WRLCK;
        fl.l_whence=SEEK_SET;
        fl.l_start = i;
        fl.l_len = 1;
        fcntl(fd, F_GETLK, &fl);

        if(fl.l_type!=F_UNLCK) {
            unsigned char byte='\0';
            lseek(fd, i, SEEK_SET);
            read(fd, &byte, 1);
            printf("Zaryglowany znak: 0x%01x nr %d\n", byte, i);
            char type[10];
            if(fl.l_type==F_RDLCK)
                strcpy(type, "Odczyt");
            else if(fl.l_type==F_WRLCK)
                strcpy(type, "Zapis");
            printf("PID: %d\nTyp: %s\n\n", fl.l_pid, type);
        }


    }
}

int main(int argc, char **argv) {

    char *fileName = argv[1];
    int fd = open(fileName, O_RDWR);
    if(fd==-1) {
        perror(fileName);
        exit(1);
    }
    if(argc!=2) {
        printf("Podano zla liczbe argumentow!\n");
        exit(1);
    }

    char c[2];
    int i;
    printf("Wpisz odpowiedni znak, aby:\n"
            "1a Ustawic rygiel do odczytu w wersji nieblokujacej\n"
            "1b Ustawic rygiel do odczytu w wersji blokujacej\n"
            "2a Ustawic rygiel do zapisu w wersji nieblokujacej\n"
            "2b Ustawic rygiel do zapisu w wersji blokujacej\n"
            "3 Wyswietlic liste zaryglowanych znakow\n"
            "4 Zwolnic wybrany rygiel\n"
            "5 Odczytac wybrany znak\n"
            "6 Zmienic wybrany znak\n"
            "0 Zakonczyc program\n");



    while(1) {
        scanf("%s", c);
        if(strcmp(c, "1a")==0 || strcmp(c, "1b")==0 || strcmp(c, "2a")==0 || strcmp(c, "2b")==0 ||
           strcmp(c, "4")==0 || strcmp(c, "5")==0 || strcmp(c, "6")==0 || strcmp(c, "7")==0) {

            printf("Podaj numer bajtu(numerowanie od 0):\n");
            scanf("%d", &i);

            if(strcmp(c, "1a")==0) {
                setLock(fd, i, 1);
            }
            else if(strcmp(c, "1b")==0) {
                setLock_block(fd, i, 1);
            }
            else if(strcmp(c, "2a")==0) {
                setLock(fd, i, 2);
            }
            else if(strcmp(c, "2b")==0) {
                setLock_block(fd, i, 2);
            }
            else if(strcmp(c, "4")==0) {
                unlock(fd, i);
            }
            else if(strcmp(c, "5")==0) {
                readByte(fd, i);
            }
            else if(strcmp(c, "6")==0) {
                unsigned char *z=malloc(sizeof(*z));
                printf("Podaj znak, ktory chcesz wpisac:\n");
                scanf("%s", z);
                writeByte(fd, i, *z);
                free(z);
            }
        }
        else if(strcmp(c, "3")==0) {
            list(fd);
        }
        else if(strcmp(c, "0")==0) {
            Node *p=first;
            while(p!=NULL) {
                unlock(fd, p->n);
                p=first;
            }
            break;
        }
        else
            printf("Podano zla liczbe!\n");
    }


    close(fd);
    return 0;

}

