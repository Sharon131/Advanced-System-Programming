#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS    10
#define READER_TURNS    10
#define READERS_COUNT   10
#define WRITERS_COUNT   10
#define OBJECTS_COUNT   10

pthread_mutex_t writers_mutexes[OBJECTS_COUNT] = { PTHREAD_MUTEX_INITIALIZER };
pthread_mutex_t readers_mutexes[OBJECTS_COUNT] = { PTHREAD_MUTEX_INITIALIZER };

unsigned int readersCount[OBJECTS_COUNT];
unsigned int readersMaxCount[OBJECTS_COUNT];

uint32_t GetRandomTime(uint32_t max_val) {
    return rand()%max_val + 1;
}

// Writer thread function
int Writer(void* data) {
    int threadId = *(int*) data;
    int i;
    // uint8_t res = 1;

    for (i = 0; i < WRITER_TURNS; i++) {
        int num = rand()%OBJECTS_COUNT;
        if (pthread_mutex_lock(&writers_mutexes[num]) != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        }
	    // Write
        printf("(W) Writer %d started writing to %d...", threadId, num);
        fflush(stdout);
        usleep(GetRandomTime(800));
        printf("writer %d finished\n", threadId);
            
        // Release ownership of the mutex object.
        if (pthread_mutex_unlock(&writers_mutexes[num]) != 0) {
            fprintf(stderr, "Error occured during unlocking the mutex.\n");
            exit (-1);
        }
        // Think, think, think, think
        usleep(GetRandomTime(1000));
    }

    free(data);
    return 0;
}

// Reader thread function
int Reader(void* data) {
    int threadId = *(int*) data;
    int i;

    for (i = 0; i < READER_TURNS; i++) {
        int num = rand()%OBJECTS_COUNT;
        bool canRead = false;
        while (!canRead) {
            if (pthread_mutex_lock(&readers_mutexes[num]) != 0) {
                fprintf(stderr, "Error occured during locking the mutex.\n");
                exit (-1);
            }
            if (readersCount[num] < readersMaxCount[num]) {
                canRead = true;
            } else {
                printf("(R) Reader %d waiting for unblocking %d by another reader.\r\n", threadId, num);
                if (pthread_mutex_unlock(&readers_mutexes[num]) != 0) {
                    fprintf(stderr, "Error occured during unlocking the mutex.\n");
                    exit (-1);
                }
            }      
        }
	    // Read
        readersCount[num]++;
        if (readersCount[num] == 1) {
            if (pthread_mutex_lock(&writers_mutexes[num]) != 0) {
                fprintf(stderr, "Error occured during locking the mutex.\n");
                exit (-1);
            }
        }            
        pthread_mutex_unlock(&readers_mutexes[num]);
        
        printf("(R) Reader %d started reading %d...", threadId, num);
        fflush(stdout);
            // Read, read, read
        usleep(GetRandomTime(200));
        printf("reader %d finished\n", threadId);
        
        if (pthread_mutex_lock(&readers_mutexes[num]) != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        }
        readersCount[num]--;
        // Release ownership of the mutex object.
        if (readersCount[num] == 0) {
            if (pthread_mutex_unlock(&writers_mutexes[num]) != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
        }
        if (pthread_mutex_unlock(&readers_mutexes[num]) != 0) {
            fprintf(stderr, "Error occured during unlocking the mutex.\n");
            exit (-1);
        }
    
        usleep(GetRandomTime(200));
    }

    free(data);

    return 0;
}

int main(int argc, char* argv[])
{
    srand(100005);

    for (int i=0;i<OBJECTS_COUNT;i++) {
        readersMaxCount[i] = 1;
    }

    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];

    int i,rc;

    // Create the Reader threads
    for (i = 0; i < READERS_COUNT; i++) {
	// Reader initialization - takes random amount of time
	    usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;
	    rc = pthread_create(
                &readerThreads[i], // thread identifier
                NULL,              // thread attributes
                (void*) Reader,    // thread function
                (void*) threadId);     // thread function argument

        if (rc != 0)
        {
    	    fprintf(stderr,"Couldn't create the reader threads");
            exit (-1);
        }
    }

    for (int i=0;i<WRITERS_COUNT;i++) {
        // Create the Writer thread
        usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;
        rc = pthread_create(
                &writerThreads[i],  // thread identifier
                NULL,           // thread attributes
                (void*) Writer, // thread function
                (void*) threadId);  // thread function argument

        if (rc != 0) 
        {
            fprintf(stderr,"Couldn't create the writer threads");
            exit (-1);
        }
    }

    // At this point, the readers and writers should perform their operations

    // Wait for the Readers
    for (i = 0; i < READERS_COUNT; i++) 
        pthread_join(readerThreads[i],NULL);

    // Wait for the Writers
    for (int i=0;i<WRITERS_COUNT;i++) {
        pthread_join(writerThreads[i],NULL);
    }

    return (0);
}