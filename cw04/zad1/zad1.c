#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int direction;

void printLetter(char *letter) {

    printf("%c\n", *letter);
    if(direction==1 && *letter!='Z')
        (*letter)++;
    else if(direction == -1 && *letter!='A')
        (*letter)--;

}

void turnOrEnd(int signum) {
    if(signum == SIGTSTP)
        direction *= -1;
    else if(signum == SIGINT) {
        printf("Odebrano sygnal SIGINT\n");
        exit(0);
    }
}

int main(void) {

    char letter = 'A';
    direction=1;
    struct sigaction act;
    act.sa_handler=turnOrEnd;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGTSTP, &act, NULL) == -1) {
        perror("Error with sigaction function");
    }
    if(signal(SIGINT, turnOrEnd) == SIG_ERR) {
        perror("Error with signal function");
    }

    while(1) {
        sleep(1);
        printLetter(&letter);
    }

    return 0;
}

