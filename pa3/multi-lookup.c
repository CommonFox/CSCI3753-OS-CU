/*
 * File: lookup.c
 * Author: Andy Sayler, Fox Maikovich
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2012/02/01 2016/14/10
 * Modify Date: 2012/02/01 2016/14/10
 * Description:
 * 	This file contains the threaded
 *      solution to this assignment.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "queue.h"
#include "util.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

queue q;
pthread_mutex_t ql;
pthread_mutex_t iol;
FILE* outputfp = NULL; // Output file, must be global in order for resolver to write to it
int finished = 0; // Check to see if threads have finished

void* request(void* threadid){
    char hostname[SBUFSIZE]; // The individual hostnames
    char* data; // URL to be pushed onto the queue

    FILE* inputfp = fopen(threadid, "r"); // Open inputfile

    /* Read the file and process it */
    while(fscanf(inputfp, INPUTFS, hostname) > 0){
        int condition = 0; // Check if the locks are done

        while(condition == 0) {
            pthread_mutex_lock(&ql); // Make sure only one can check on queue at a time
            if(queue_is_full(&q)) // Check if queue is full
            {
                pthread_mutex_unlock(&ql);
                // Sleep for a few seconds, wait for resolver thread to take something of the queue
                usleep((rand()%100)*10000+1000000); 
            }
            else
            {
                data = malloc(SBUFSIZE); // Allocate memory for the data
                strncpy(data, hostname, SBUFSIZE);
                queue_push(&q, data); // Push data to queue
                pthread_mutex_unlock(&ql); // Unlock to allow next thread to access the queue
                condition = 1;
            }
        }
	}

	/* Close Input File */
	fclose(inputfp);
    return NULL;
}

void* resolve(){
    char* hostname; // Contains the hostname
    char firstipstr[INET6_ADDRSTRLEN]; //Contains resolved IP address

    while(!queue_is_empty(&q) || !finished) { // finished determines if you have read all files or if the queue is not empty
            pthread_mutex_lock(&ql); // lock the queue, each thread cannot point to the same item on the queue
            if(!queue_is_empty(&q)) // check if queue is empty
            {
                // take the data off of the queue
                hostname = queue_pop(&q);

                if(hostname != NULL){
                    pthread_mutex_unlock(&ql); // unlock so next thread can use the queue

                    // Check if it is a proper address
                    if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
                       == UTIL_FAILURE){
                        fprintf(stderr, "dnslookup error: %s\n", hostname);
                        strncpy(firstipstr, "", sizeof(firstipstr));
                    }

                    // If not lock the file and write to it
                    pthread_mutex_lock(&iol);
                    fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
                    pthread_mutex_unlock(&iol); // unlock the file for next thread
                }
                
                free(hostname);
            }
            else{
                // This means that the queue is empty
                pthread_mutex_unlock(&ql);
            }

    }
    return NULL;
}

int main(int argc, char* argv[]){
    /* Local Vars */
    
    pthread_mutex_init(&ql, NULL);
    pthread_mutex_init(&iol, NULL);

    queue_init(&q, 1000);

    pthread_t requesterThreads[argc-1]; //one for each file passed in as agrument
    pthread_t resolverThreads[10]; // Max amount of resolver threads

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* Check Arguments */
    if(argc < MINARGS){
        fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
        fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
        return EXIT_FAILURE;
    }
    if(argc > 10){
        fprintf(stderr, "Too many files: %d\n", (argc -2));
        fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
        return EXIT_FAILURE;
    }

        /* test if valid input and output files */
    int valid;
    for(int i=1; i<(argc-2); i++){
        valid = access(argv[i], F_OK);
        if(valid == 0)
        {
            break;
        }
        else
        {
            fprintf(stderr, "Invalid input files: %d\n", (argc -2));
            return EXIT_FAILURE;
        }
	}

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
	    perror("Error Opening Output File");
	    return EXIT_FAILURE;
    }

    /* Loop Through Input Files */
    for(int i=1; i<(argc-1); i++){
        // each file gets a requester thread
        int thr = pthread_create(&requesterThreads[i-1], &attr, request, argv[i]);
        if(thr) {
            printf("Request thread error\n");
        }
	}

	for(int i = 0; i < 10; ++i){
        // create 10 resolver threads
        int thr = pthread_create(&resolverThreads[i], &attr, resolve, NULL);
        if (thr) {
            printf("Resolver thread error\n");
        }
	}

	/* Join threads to signal if they're done, wait for threads to finish */
	for(int i = 0; i < (argc-2); ++i)
    {
    	int rc = pthread_join( requesterThreads[i],  NULL);
    	if(rc)
    	{
    		printf("Request thread broke");
    	}
    }
    finished = 1;

    /* Join on the resolver threads */
    for(int i = 0; i < 10; ++i)
    {
    	int rc = pthread_join( resolverThreads[i], NULL);
		if(rc)
    	{
    		printf("Resolver thread broke");
    	}
    }

	/* Close Input File */
	//fclose(inputfp);

    /* Close Output File */
    fclose(outputfp);

    queue_cleanup(&q);
    pthread_mutex_destroy(&ql);
    pthread_mutex_destroy(&iol);

    return EXIT_SUCCESS;
}
