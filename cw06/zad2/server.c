#include <stdio.h>
#include <time.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "cs.h"

int clients[10][2], i;
mqd_t qd;

int find_client(int pid, int clients[10][2]) {

    for(int i=0; i<10; i++) {
        if(clients[i][1]==pid)
            return i;
    }
    return -1;
}

void copy(char *msg, char *msg_to_client, char opt) {

    int k;
    if(opt==TIME) {
        time_t t = time(NULL);

        char time[30];
        strcpy(time, asctime(localtime(&t)));
        for(k=0; k<MAXSIZE-1 && time[k]!='\n'; k++) {
            msg_to_client[k+1] = time[k];
        }
        msg_to_client[k+1]='\0';

    }
    else {
        for (k = 1; k<MAXSIZE-1 && msg[k] != '\n'; k++) {
            if (opt == CAPS && msg[k] >= 'a') {
                msg[k] -= 32;
            }
            msg_to_client[k] = msg[k];
        }
        msg_to_client[k] = '\0';
    }
}

void delete_queue(void) {
    for(int j=0; j<i-1; j++) {
        mq_close(clients[j][0]);
    }
    mq_close(qd);
    mq_unlink(SERVER);
}

void end(int signum) {
    exit(0);
}

int main(void) {

    char msg[MAXSIZE], msg_to_client[MAXSIZE];
    unsigned int prio;
    int pid, clients[10][2], c, b=0;

    struct sigaction act;
    act.sa_handler = end;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    atexit(&delete_queue);

    struct mq_attr mqattr;
    mqattr.mq_msgsize=MAXSIZE;
    mqattr.mq_maxmsg = 10;
    qd = mq_open(SERVER, O_RDWR | O_CREAT, 0600, &mqattr);
    if(qd == -1) {
        perror("Error with mq_open");
        return 1;
    }

    i=1;
    while(1) {
        if(mq_receive(qd, msg, MAXSIZE, &prio)==-1) {
            perror("Error with mq_receive");
            return 1;
        }

        pid = prio;

        if(msg[0]==ECHO || msg[0]==CAPS || msg[0]==TIME) {
            c = find_client(pid, clients);
            msg_to_client[0]=(char) (c + 1 + '0');

            copy(msg, msg_to_client, msg[0]);

            printf("%s\n", msg_to_client);
            if(mq_send(clients[c][0], msg_to_client, MAXSIZE, 1)==-1) {
                perror("Error with mq_send");
                return 1;
            }
        }
        else if(msg[0]==END) {
            b=1;
        }
        else {
            mqd_t client_qd = mq_open(msg, O_RDWR);
            if(client_qd == -1) {
                perror("Error with client mq_open");
                return 1;
            }

            clients[i-1][0] = client_qd;
            clients[i-1][1] = pid;

            msg_to_client[0]=(char) (i + '0');
            msg_to_client[1]='\0';
            i++;
            if(mq_send(client_qd, msg_to_client, MAXSIZE, 1)==-1) {
                perror("Error with mq_send");
                return 1;
            }
        }

        if(b) {
            struct mq_attr attr;
            if(mq_getattr(qd, &attr)==-1) {
                perror("Error with mq_setattr");
                return 1;
            }
            if(attr.mq_curmsgs==0) {
                for(int j=0; j<i-1; j++) {
                    kill(clients[j][1], SIGINT);
                }
                return 0;
            }
        }
    }

}

