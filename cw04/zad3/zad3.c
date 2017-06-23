#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int n, childPid, L;

void countSignalsChild(int signum) {

    n++;
    if(signum==SIGUSR2 || signum==SIGRTMIN+10) {
        printf("Liczba odebranych sygnalow przez potomka: %d\n", n);
    }
}

void countSignals(int signum) {

    if(signum == SIGINT) {
        kill(childPid, SIGKILL);
    }
    else {
        n++;
    }
}

int main(int argc, char **argv) {

    if(argc!=3) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    int Type = atoi(argv[2]);
    L = atoi(argv[1]);
    n=0;

    int pid = fork();
    if(pid == 0) {
        struct sigaction act;
        act.sa_handler = countSignalsChild;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
        sigaction(SIGRTMIN + 17, &act, NULL);
        sigaction(SIGRTMIN + 10, &act, NULL);

        sigset_t set, old_set, wait_set;
        sigemptyset(&set);
        sigemptyset(&old_set);
        sigaddset(&set, SIGUSR2);
        sigaddset(&set, SIGRTMIN + 10);
        sigprocmask(SIG_BLOCK, &set, &old_set);

        pause();
        do {
            sigpending(&wait_set);
        } while(sigismember(&wait_set, SIGUSR1) || sigismember(&wait_set, SIGRTMIN+17));

        for(int i=0; i<L; i++) {
            if(Type==3) {
                kill(getppid(), SIGRTMIN+17);
            }
            else {
                kill(getppid(), SIGUSR1);
            }
        }

        sigsuspend(&old_set);
        return 0;
    }
    else if(pid > 0) {
        sleep(1);
        childPid=pid;
        struct sigaction act;
        act.sa_handler = countSignals;
        sigemptyset(&act.sa_mask);
        act.sa_flags=SA_RESTART;
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGRTMIN+17, &act, NULL);

        int i;
        union sigval val;
        val.sival_int = L;
        for (i=0; i <= L; i++) {
            if(Type==1) {
                if(i==L) {
                    kill(pid, SIGUSR2);
                }
                else {
                    kill(pid, SIGUSR1);
                }

            }
            else if(Type==2) {
                if(i==L) {
                    sigqueue(pid, SIGUSR2, val);
                }
                else {
                    sigqueue(pid, SIGUSR1, val);
                }
            }
            else {
                if(i==L) {
                    kill(pid, SIGRTMIN+10);
                }
                else {
                    kill(pid, SIGRTMIN+17);
                }
            }
        }
        int status;
        wait(&status);
        printf("Liczba wyslanych sygnalow do potomka: %d\n", i);
        printf("Liczba odebranych sygnalow od potomka: %d\n", n);
    }
    else {
        perror("Error with fork");
        return 1;
    }

    return 0;
}

