#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS    10
#define READER_TURNS    10
#define READERS_COUNT   10

int readersCount;

sem_t readers_semaphore;
sem_t writers_semaphore;

uint32_t GetRandomTime(uint32_t max_val) {
    return rand()%max_val + 1;
}

// Writer thread function
int Writer(void* data) {
    int i;
    
    for (i = 0; i < WRITER_TURNS; i++) {
        int result = sem_wait(&writers_semaphore);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	    // Write
            printf("(W) Writer started writing...");
            fflush(stdout);
            usleep(GetRandomTime(800));
            printf("writer finished\n");
                
            // Release ownership of the mutex object.
            result = sem_post(&writers_semaphore);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
            // Think, think, think, think
            usleep(GetRandomTime(1000));
        }
    }

    return 0;
}

// Reader thread function
int Reader(void* data) {
    int i;
    int threadId = *(int*) data;
    
    for (i = 0; i < READER_TURNS; i++) {
        int result = sem_wait(&readers_semaphore);

        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	    // Read
            readersCount++;
            if (readersCount == 1) {
                sem_wait(&writers_semaphore);
            }
            sem_post(&readers_semaphore);
            
            printf("(R) Reader %d started reading...", threadId);
            fflush(stdout);
                // Read, read, read
            usleep(GetRandomTime(200));
            printf("reader finished\n");
                
            // Release ownership of the mutex object.
            result = sem_wait(&readers_semaphore);
            readersCount--;
            if (readersCount == 0) {
                sem_post(&writers_semaphore);
            }
            result = sem_post(&readers_semaphore);
            if (result != 0) {
                fprintf(stderr, "Error occured during unlocking the mutex.\n");
                exit (-1);
            }
	    
            usleep(GetRandomTime(800));
        }
    }

    free(data);

    return 0;
}

int main(int argc, char* argv[])
{
    srand(100005);
    
    sem_init(&readers_semaphore, 0, 1);
    sem_init(&writers_semaphore, 0, 1);

    pthread_t writerThread;
    pthread_t readerThreads[READERS_COUNT];

    int i,rc;

    // Create the Writer thread
    rc = pthread_create(
            &writerThread,  // thread identifier
            NULL,           // thread attributes
            (void*) Writer, // thread function
            (void*) NULL);  // thread function argument

    if (rc != 0) 
    {
    	fprintf(stderr,"Couldn't create the writer thread");
        exit (-1);
    }

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

    // At this point, the readers and writers should perform their operations

    // Wait for the Readers
    for (i = 0; i < READERS_COUNT; i++) 
        pthread_join(readerThreads[i],NULL);

    // Wait for the Writer
    pthread_join(writerThread,NULL);

    return (0);
}