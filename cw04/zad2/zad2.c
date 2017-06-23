#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

int requests, K, N, n;
pid_t *pidsToUnlock, *pids;

void randSig(int signum){

    n = rand()%31;
}

void processSignal(int signum, siginfo_t *siginfo, void *uc) {
    pid_t pid = siginfo->si_pid;
    if(signum == SIGUSR1) {
        printf("Signal: SIGUSR1, Child PID: %d\n", (int) pid);
        if(requests<K) {
            pidsToUnlock[requests] = pid;
            requests++;
        }
        if(requests==K) {
            for(int i=0; i<K; i++) {
                if(kill(pidsToUnlock[i], SIGALRM)==-1) {
                    perror("Error with kill");
                    exit(1);
                }
            }
            requests++;
        }
        else if(requests>K) {
            if(kill(pid, SIGALRM)==-1) {
                perror("Error with kill");
                exit(1);
            }
        }
    }
    else if(signum==SIGINT){
        for(int i=0; i<N; i++) {
            if(pids[i]!=-1) {
                kill(pids[i], SIGKILL);
                printf("Process %d killed\n", pids[i]);
            }
        }
    }
    else {
        for(int i=0; i<31; i++) {
            if(signum==SIGRTMIN+i) {
                if(i==30)
                    printf("Signal: SIGRTMAX, Child PID: %d, exit status: %d\n", (int) pid, siginfo->si_value.sival_int);
                else
                    printf("Signal: SIGRTMIN+%d, Child PID: %d, exit status: %d\n", i, (int) pid, siginfo->si_value.sival_int);
            }
        }
        for(int i=0; i<N; i++) {
            if(pids[i]==pid) pids[i]=-1;
        }
    }
}

int code(struct timeval t1, struct timeval t2) {
    int c = (int) ((t2.tv_sec - t1.tv_sec)*1000 + (t2.tv_usec-t1.tv_usec)/1000000);
    printf("Exit status of process %d: %d\n", getpid(), c);
    return c;
}


int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    N = atoi(argv[1]);
    K = atoi(argv[2]);
    requests = 0;
    pids = malloc(N * sizeof(pid_t));
    pidsToUnlock = malloc(K * sizeof(pid_t));

    for (int i = 0; i < N; i++) {

        if ((pids[i] = fork()) == 0) {
            srand((unsigned) time(NULL) ^ (getpid()<<16));
            struct sigaction act;
            struct timeval fTime;
            struct timeval sTime;
            act.sa_handler = randSig;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            if (sigaction(SIGALRM, &act, NULL) == -1) {
                perror("Error with sigaction");
                exit(1);
            }
            int s = rand()%10+1;
            printf("%d Zasypiam na %d sek\n", getpid(), s);
            sleep((unsigned) s);

            gettimeofday(&fTime, NULL);
            if (kill(getppid(), SIGUSR1) == -1) {
                perror("Error with kill");
                exit(1);
            }

            pause();

            gettimeofday(&sTime, NULL);
            int c = code(fTime, sTime);
            union sigval val;
            val.sival_int=c;
            if(sigqueue(getppid(), SIGRTMIN+n, val)==-1) {
                perror("Error with kill");
                exit(1);
            }
            return c;
        }
        else if(pids[i]<0) {
            perror("Error with fork");
            exit(1);
        }
    }

    struct sigaction act;
    act.sa_sigaction = processSignal;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    for(int i=0; i<31; i++) {
        sigaddset(&act.sa_mask, SIGRTMIN+i);
    }
    act.sa_flags = SA_SIGINFO | SA_RESTART;  //SA_RESTART w celu ponawiania przerwania systemowego wait
    if(sigaction(SIGUSR1, &act, NULL)==-1) {
        perror("Error with sigaction - parent");
        exit(1);
    }
    for(int i=0; i<31; i++) {
        if(sigaction(SIGRTMIN+i, &act, NULL)==-1) {
            perror("Error with sigaction - parent");
            exit(1);
        }
    }
    if(sigaction(SIGINT, &act, NULL)==-1) {
        perror("Error with sigaction - parent");
        exit(1);
    }

    int status;
    for(int i=0; i<N; i++) {
        pid_t pid = wait(&status);
        if(pid == -1) {
            perror("Error with wait");
            break;
        }
    }

    free(pids);
    free(pidsToUnlock);
    return 0;
}

