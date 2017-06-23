#include <stdio.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "cs.h"

int msgque;

int find_client(int pid, int clients[10][2]) {

    for(int i=0; i<10; i++) {
        if(clients[i][1]==pid)
            return i;
    }
    return -1;
}

void copy(mymsg *msg_client, mymsg *my_msg, long option) {

    int k;
    if(option==TIME) {
        time_t t = time(NULL);

        char time[30];
        strcpy(time, asctime(localtime(&t)));
        for(k=0; k<sizeof(my_msg->mtext) && time[k]!='\n'; k++) {
            my_msg->mtext[k] = time[k];
        }
        my_msg->mtext[k]='\0';

    }
    else {
        for (k = 0; k<sizeof(my_msg->mtext) && msg_client->mtext[k] != '\n'; k++) {
            if (option == CAPS && msg_client->mtext[k] >= 'a') {
                msg_client->mtext[k] -= 32;
            }
            my_msg->mtext[k] = msg_client->mtext[k];
        }
        my_msg->mtext[k] = '\0';
    }
}

void delete_queue(void) {
    msgctl(msgque, IPC_RMID, NULL);
}

void end(int signum) {
    exit(0);
}

int main(void) {

    int clients[10][2], i, b, client_msgque, c_pid, c;
    struct mymsg msg_client, my_msg;
    char *homedir;

    struct sigaction act;
    act.sa_handler = end;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    homedir = getenv("HOME");
    atexit(&delete_queue);

    key_t key = ftok(homedir, PROJ);
    if(key==-1) {
        perror("Error with ftok");
        return 1;
    }

    if((msgque=msgget(key, IPC_CREAT | 0600))==-1) {
        perror("Error with msgget");
        return 1;
    }

    b=0;
    i=1;
    while(1) {
        if (msgrcv(msgque, &msg_client, MAXSIZE, 0, 0) == -1) {
            perror("Error with msgrcv");
            return 1;
        }

        c_pid = msg_client.mpid;

        if(msg_client.mtype==ECHO || msg_client.mtype==CAPS || msg_client.mtype==TIME) {
            c = find_client(c_pid, clients);
            my_msg.mtype = c+1;

            copy(&msg_client, &my_msg, msg_client.mtype);

            printf("%ld: %s\n", my_msg.mtype, my_msg.mtext);
            if(msgsnd(clients[c][0], &my_msg, MAXSIZE, 0)==-1) {
                perror("Error with msgsnd");
                return 1;
            }
        }
        else if(msg_client.mtype==END) {
            b=1;
        }
        else {
            client_msgque = msgget((key_t) msg_client.mtype, 0);
            clients[i-1][0] = client_msgque;
            clients[i-1][1] = c_pid;

            my_msg.mtype = i;
            i++;
            if(msgsnd(client_msgque, &my_msg, MAXSIZE, 0) == -1) {
                perror("Error with msgsnd");
                return 1;
            }
        }

        if(b) {
            struct msqid_ds buf;
            msgctl(msgque, IPC_STAT, &buf);
            if(buf.msg_qnum==0) {
                for(int j=0; j<i-1; j++) {
                    kill(clients[j][1], SIGINT);
                }
                return 0;
            }

        }

    }
}

