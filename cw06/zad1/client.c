#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include "cs.h"

int msgque;

int set_key(char *homedir) {

    int my_key;
    int n = rand()%10;

    switch(n) {
        case 0:
            my_key = ftok(homedir, PROJ2);
            break;
        case 1:
            my_key = ftok(homedir, PROJ3);
            break;
        case 2:
            my_key = ftok(homedir, PROJ4);
            break;
        case 3:
            my_key = ftok(homedir, PROJ5);
            break;
        case 4:
            my_key = ftok(homedir, PROJ6);
            break;
        case 5:
            my_key = ftok(homedir, PROJ7);
            break;
        case 6:
            my_key = ftok(homedir, PROJ8);
            break;
        case 7:
            my_key = ftok(homedir, PROJ9);
            break;
        case 8:
            my_key = ftok(homedir, PROJ10);
            break;
        default:
            my_key = ftok(homedir, PROJ11);
    }
    return my_key;
}

void delete_queue(void) {
    msgctl(msgque, IPC_RMID, NULL);
}

void end(int signum) {
    exit(0);
}

int main(void) {

    srand((unsigned) time(NULL));

    printf("Press:\n1 - Echo\n2 - Capital letters\n3 - Time\n4 - End\n");

    key_t my_key, server_key;
    int server_msgque, c;
    struct mymsg msg;
    char *homedir;

    struct sigaction act;
    act.sa_handler = end;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    homedir = getenv("HOME");
    atexit(&delete_queue);

    my_key = set_key(homedir);
    if(my_key==-1) {
        perror("Error with ftok");
        return 1;
    }
    if((server_key=ftok(homedir, PROJ))==-1) {
        perror("Error with ftok");
        return 1;
    }
    while((msgque=msgget(my_key, IPC_CREAT | IPC_EXCL | 0600))==-1) {
        if(errno==EEXIST) {
            my_key = set_key(homedir);
        }
        else {
            perror("Error with msgget");
            return 1;
        }
    }
    if((server_msgque=msgget(server_key, 0))==-1) {
        perror("Error with msgget");
        return 1;
    }

    msg.mtype = my_key;
    msg.mpid = getpid();
    msgsnd(server_msgque, &msg, MAXSIZE, 0);

    if(msgrcv(msgque, &msg, MAXSIZE, 0, 0)==-1) {
        perror("Error with msgrcv");
        return 1;
    }
    printf("Identifier: %ld\n", msg.mtype);

    int a;
    do {
        c=getc(stdin);
        while((a = getc(stdin)) !='\n' && a != EOF);

        msg.mpid = getpid();

        if(c=='1' || c=='2') {
            if(c=='1') msg.mtype=ECHO;
            else msg.mtype=CAPS;

            printf("Enter string:\n");
            fgets(msg.mtext, sizeof(msg.mtext), stdin);

            if(msgsnd(server_msgque, &msg, MAXSIZE, 0)==-1) {
                perror("Error with msgsnd");
                return 1;
            }

            msgrcv(msgque, &msg, MAXSIZE, 0, 0);
            printf("%s\n", msg.mtext);
        }
        else if (c=='3') {
            msg.mtype=TIME;
            if(msgsnd(server_msgque, &msg, MAXSIZE, 0)==-1) {
                perror("Error with msgsnd");
                return 1;
            }

            msgrcv(msgque, &msg, MAXSIZE, 0, 0);
            printf("%s\n", msg.mtext);
        }
        else {
            msg.mtype = END;
            if(msgsnd(server_msgque, &msg, MAXSIZE, 0)==-1) {
                perror("Error with msgsnd");
                return 1;
            }
        }

    } while(c=='1' || c=='2' || c=='3');

    return 0;
}

