/*
 * File: mixed.c
 * Author: Fox Maikovich
 * Adapted From: Andy Sayler
 * Revised: Shivakant Mishra
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: 2012/30/10
 * Modify Date: 2016/30/10
 * Description: A small i/o bound program to copy N bytes from an input
 *              file to an output file. May read the input file multiple
 *              times if N is larger than the size of the input file.
 */

/* Include Flags */
#define _GNU_SOURCE

/* System Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <math.h>

/* Local Defines */
#define MAXFILENAMELENGTH 80
#define DEFAULT_INPUTFILENAME "rwinput"
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput"
#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_TRANSFERSIZE 1024*100

#define DEFAULT_pro 1000000
#define RADIUS (RAND_MAX / 2)

static inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

static inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){

    int rv;
    int inputFD;
    int outputFD;
    char inputFilename[MAXFILENAMELENGTH];
    char outputFilename[MAXFILENAMELENGTH];
    char outputFilenameBase[MAXFILENAMELENGTH];

    ssize_t transfersize = 0;
    ssize_t blocksize = 0; 
    char* transferBuffer = NULL;
    ssize_t buffersize;

    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    int totalReads = 0;
    ssize_t bytesWritten = 0;
    ssize_t totalBytesWritten = 0;
    int totalWrites = 0;
    int inputFileResets = 0;

    /* Variables from pi-sched.c */
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

	/* Set up defaults since arguments have been reworked */
	blocksize = DEFAULT_BLOCKSIZE;
	transfersize = DEFAULT_TRANSFERSIZE;
	strncpy(inputFilename, DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH);
	strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);
    
	/* Set number of processes */
    if(argc < 2) {
		exit(EXIT_FAILURE);
	} else {
		pro = atoi(argv[1]);
		if(pro < 1){
			fprintf(stderr, "Need more processes\n");
			exit(EXIT_FAILURE);
		} 
	}

	/* Set policy if supplied */
	if(argc < 3){
		policy = SCHED_OTHER;
	} else {
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

	/* Set process to max priority for given scheduler */
	param.sched_priority = sched_get_priority_max(policy);

	/* Set new scheduler policy */
	fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
	fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
	if(sched_setscheduler(0, policy, &param)){
		perror("Error setting scheduler policy");
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

	/* Begin forking and testing process */
	fprintf(stdout, "Creating %d simultanious processes\n", pro);
	for(int i = 0; i < pro; i++) {
		if ((pid = fork()) == -1) {
			fprintf(stderr, "The child process %d could not be forked\n", i);
			exit(EXIT_FAILURE);
		}
		else if (pid == 0) {
			fprintf(stdout, "Process %d forked with PID %d\n", i, pid);
			
			/* Confirm blocksize is multiple of and less than transfersize*/
			if(blocksize > transfersize){
			fprintf(stderr, "blocksize can not exceed transfersize\n");
			exit(EXIT_FAILURE);
			}
			if(transfersize % blocksize){
			fprintf(stderr, "blocksize must be multiple of transfersize\n");
			exit(EXIT_FAILURE);
			}

			/* Allocate buffer space */
			buffersize = blocksize;
			if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
			perror("Failed to allocate transfer buffer");
			exit(EXIT_FAILURE);
			}
			
			/* Open Input File Descriptor in Read Only mode */
			if((inputFD = open(inputFilename, O_RDONLY | O_SYNC)) < 0){
			perror("Failed to open input file");
			exit(EXIT_FAILURE);
			}

			/* Open Output File Descriptor in Write Only mode with standard permissions*/
			rv = snprintf(outputFilename, MAXFILENAMELENGTH, "%s-%d",
				outputFilenameBase, getpid());    
			if(rv > MAXFILENAMELENGTH){
			fprintf(stderr, "Output filenmae length exceeds limit of %d characters.\n",
				MAXFILENAMELENGTH);
			exit(EXIT_FAILURE);
			}
			else if(rv < 0){
			perror("Failed to generate output filename");
			exit(EXIT_FAILURE);
			}
			if((outputFD =
			open(outputFilename,
				O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
			perror("Failed to open output file");
			exit(EXIT_FAILURE);
			}

			/* Print Status */
			fprintf(stdout, "Reading from %s and writing to %s\n",
				inputFilename, outputFilename);

			/* Read from input file and write to output file*/
			do{
			/* Read transfersize bytes from input file*/
			bytesRead = read(inputFD, transferBuffer, buffersize);
			if(bytesRead < 0){
				perror("Error reading input file");
				exit(EXIT_FAILURE);
			}
			else{
				totalBytesRead += bytesRead;
				totalReads++;
			}

			/* If all bytes were read, write to output file*/
			if(bytesRead == blocksize){
				bytesWritten = write(outputFD, transferBuffer, bytesRead);
				if(bytesWritten < 0){
				perror("Error writing output file");
				exit(EXIT_FAILURE);
				}
				else{
				totalBytesWritten += bytesWritten;
				totalWrites++;
				}
			}
			/* Otherwise assume we have reached the end of the input file and reset */
			else{
				if(lseek(inputFD, 0, SEEK_SET)){
				perror("Error resetting to beginning of file");
				exit(EXIT_FAILURE);
				}
				inputFileResets++;
			}
			
			}while(totalBytesWritten < transfersize);

			/* Output some possibly helpfull info to make it seem like we were doing stuff */
			fprintf(stdout, "Read:    %zd bytes in %d reads\n",
				totalBytesRead, totalReads);
			fprintf(stdout, "Written: %zd bytes in %d writes\n",
				totalBytesWritten, totalWrites);
			fprintf(stdout, "Read input file in %d pass%s\n",
				(inputFileResets + 1), (inputFileResets ? "es" : ""));
			fprintf(stdout, "Processed %zd bytes in blocks of %zd bytes\n",
				transfersize, blocksize);
			
			/* Free Buffer */
			free(transferBuffer);

			/* Close Output File Descriptor */
			if(close(outputFD)){
			perror("Failed to close output file");
			exit(EXIT_FAILURE);
			}

			/* Close Input File Descriptor */
			if(close(inputFD)){
			perror("Failed to close input file");
			exit(EXIT_FAILURE);
			}

			/* Implment the pi calculation */
            for(int j=0; j<it; j++){
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

	fprintf(stdout, "\nWaiting for all of the running processes to exit\n");
	int reap = 0;
	while ((pid = wait(&test)) > 0 ){
		if(WIFEXITED(test)) {
			fprintf(stdout, "Child process %d has been reaped correctly.\n", pid);
			reap++;
		}
	}
	if (reap == pro) {
		fprintf(stdout, "%d child process forked, %d have been reaped.\n", pro, reap);
		exit(EXIT_SUCCESS);
	}
	else {
		fprintf(stderr, "Something went wrong.\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
