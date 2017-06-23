#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "cs.h"

mqd_t qd, qd_server;
char queue[6];

void delete_queue(void) {
    mq_close(qd);
    mq_close(qd_server);
    mq_unlink(queue);
}

void end(int signum) {
    exit(0);
}

int main(void) {

    srand((unsigned) time(NULL));
    char msg[MAXSIZE];
    unsigned int prio;
    printf("Press:\n1 - Echo\n2 - Capital letters\n3 - Time\n4 - End\n");

    struct sigaction act;
    act.sa_handler = end;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT, &act, NULL);

    atexit(&delete_queue);

    queue[0] = '/';
    for(int i=1; i<6; i++) {
        queue[i] =(char) (rand()%26 + 97);
    }
    struct mq_attr mqattr;
    mqattr.mq_msgsize=MAXSIZE;
    mqattr.mq_maxmsg = 10;
    qd = mq_open(queue, O_RDWR | O_CREAT, 0600, &mqattr);
    if(qd == -1) {
        perror("Error with mq_open");
        return 1;
    }
    qd_server = mq_open(SERVER, O_RDWR);
    if(qd_server==-1) {
        perror("Error with mq_open");
        return 1;
    }

    strcpy(msg, queue);
    prio =(unsigned int) getpid();
    if(mq_send(qd_server, msg, MAXSIZE, prio)==-1) {
        perror("Error with mq_send");
        return 1;
    }

    if(mq_receive(qd, msg, MAXSIZE, &prio)==-1) {
        perror("Error with mq_receive");
        return 1;
    }
    printf("Identifier: %s\n", msg);

    int a, c;
    do {
        c=getc(stdin);
        while((a = getc(stdin)) !='\n' && a != EOF);

        prio =(unsigned int) getpid();

        if(c=='1' || c=='2') {
            if(c=='1') msg[0]=ECHO;
            else msg[0]=CAPS;

            printf("Enter string:\n");
            fgets(msg+1, MAXSIZE-1, stdin);

            if(mq_send(qd_server, msg, MAXSIZE, prio)==-1) {
                perror("Error with mq_send");
                return 1;
            }

            mq_receive(qd, msg, MAXSIZE, &prio);
            printf("%s\n", msg+1);
        }
        else if (c=='3') {
            msg[0]=TIME;
            if(mq_send(qd_server, msg, MAXSIZE, prio)==-1) {
                perror("Error with mq_send");
                return 1;
            }

            mq_receive(qd, msg, MAXSIZE, &prio);
            printf("%s\n", msg+1);
        }
        else {
            msg[0] = END;
            if(mq_send(qd_server, msg, MAXSIZE, prio)==-1) {
                perror("Error with mq_send");
                return 1;
            }
        }

    } while(c=='1' || c=='2' || c=='3');

    return 0;

}
