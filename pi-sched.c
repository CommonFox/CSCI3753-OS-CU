/*
 * File: pi-sched.c
 * Author: Andy Sayler
 * Revised: Dhivakant Mishra
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: 2012/03/07
 * Modify Date: 2012/03/09
 * Modify Date: 2016/31/10
 * Description:
 * 	This file contains a simple program for statistically
 *      calculating pi using a specific scheduling policy.
 */

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define DEFAULT_pro 1000000
#define RADIUS (RAND_MAX / 2)

static inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

static inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){

    long it = 1000000;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;
    
    pid_t pid;
    int test;
    int pro;
    int policy;
    struct sched_param param;

    /* Process program arguments to select pro and policy */
    /* Set default pro if not supplied */
    if(argc < 2){
	pro = DEFAULT_pro;
    }

    /* Set default policy if not supplied */
    if(argc < 3){
	policy = SCHED_OTHER;
    }

    /* Set processes if supplied */
    if(argc > 1){
        pro = atol(argv[1]);
        if(pro < 1){
            fprintf(stderr, "Bad pro value\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Set policy if supplied */
    if(argc > 2){
        if(!strcmp(argv[2], "SCHED_OTHER")){
            policy = SCHED_OTHER;
	}
        else if(!strcmp(argv[2], "SCHED_FIFO")){
            policy = SCHED_FIFO;
	}
        else if(!strcmp(argv[2], "SCHED_RR")){
            policy = SCHED_RR;
	}
        else{
            fprintf(stderr, "Unhandeled scheduling policy\n");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Set new scheduler policy */
    fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
    fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);

    if(sched_setscheduler(0, policy, &param)){
	perror("Error setting scheduler policy");
	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

    /* Start testing and create the other processes*/
    fprintf(stdout, "Creating %d simultanious processes\n", pro);
    for(int i = 0; i < pro; i++) { 
        if ((pid = fork()) == -1) { //check if child creation was unsuccessful
            fprintf(stderr, "The child process %d could not be forked\n", i);
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            fprintf(stdout, "Process %d correctly forked with PID %d\n", i , pid);

            /* Set process to max prioty for given scheduler */
            if (i % 2 == 0) {
                param.sched_priority = sched_get_priority_max(policy);
            }
            else {
                param.sched_priority = sched_get_priority_min(policy);
            }

            /* Calculate pi using statistical methode across all pro*/
            for(i=0; i<it; i++){
                x = (random() % (RADIUS * 2)) - RADIUS;
                y = (random() % (RADIUS * 2)) - RADIUS;
                if(zeroDist(x,y) < RADIUS){
                    inCircle++;
                }
                inSquare++;
            }

            /* Finish calculation */
            pCircle = inCircle/inSquare;
            piCalc = pCircle * 4.0;

            /* Print result */
            fprintf(stdout, "pi = %f\n", piCalc);

            exit(0);
        }
    }

    /* Reap all of the forked processes */
    fprintf(stdout, "\nWaiting for all of the running processes to exit\n");
    int reap = 0;
    while ((pid = wait(&test)) > 0) { //check if there are still process that need to be reaped, takes address returns pid
        if(WIFEXITED(test)) { //will return non zero if child terminated normally
            //fprintf(stdout, "Child process %d has been reaped correctly.\n", pid);
            reap++;
        }
    }
    if (reap == pro) {
        //fprintf(stdout, "%d child process forked, %d have been reaped.\n", pro, reap);
        exit(EXIT_SUCCESS);
    }
    else {
        //fprintf(stderr, "Something went wrong.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
