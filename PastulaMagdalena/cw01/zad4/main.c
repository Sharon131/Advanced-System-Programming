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

bool was_reviewed[WRITERS_COUNT];

pthread_mutex_t writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t critic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readers_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t critic_cond = PTHREAD_COND_INITIALIZER;

int readersCount;
int writersCount;

uint32_t GetRandomTime(uint32_t max_val) {
    return rand()%max_val + 1;
}

// Writer thread function TODO
int Writer(void* data) {
    int threadId = *(int*) data;
    
    for (int i = 0; i < WRITER_TURNS; i++) {
        if (pthread_mutex_lock(&critic_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the critic mutex.\n");
            exit (-1);
        }
        //Write
        printf("(W) Writer %d started writing to critic...", threadId);
        fflush(stdout);
        usleep(GetRandomTime(200));
        writersCount++;
        printf("writer finished\n");

        if (writersCount >= WRITERS_COUNT) {
            pthread_cond_broadcast(&critic_cond);
        }
        if (pthread_mutex_unlock(&critic_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the critic mutex.\n");
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
    
    for (int i = 0; i < READER_TURNS; i++) {
        int result = pthread_mutex_lock(&readers_mutex);
        if (result != 0) {
            fprintf(stderr, "Error occured during locking the mutex.\n");
            exit (-1);
        } else {
	        // Read
            readersCount++;
            if (readersCount == 1) {
                pthread_mutex_lock(&writer_mutex);
            }
            pthread_mutex_unlock(&readers_mutex);
            
            printf("(R) Reader %d started reading...", threadId);
            fflush(stdout);
                // Read, read, read
            usleep(GetRandomTime(200));
            printf("reader finished\n");
                
            // Release ownership of the writing semaphore
            result = pthread_mutex_lock(&readers_mutex);
            readersCount--;
            if (readersCount == 0) {
                pthread_mutex_unlock(&writer_mutex);
            }
            pthread_mutex_unlock(&readers_mutex);
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

int Critic(void* data) {
    
    while (1) {
        if (pthread_mutex_lock(&critic_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the critic mutex.\n");
            exit (-1);
        }
        while (writersCount < WRITERS_COUNT) {
            pthread_cond_wait(&critic_cond, &critic_mutex);
            printf("(C) Critic waiting for writers.\r\n");
        }
        if (pthread_mutex_lock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the writer mutex.\n");
            exit (-1);
        }
        
        printf("(C) Critic started reviewing and writing...");
        fflush(stdout);
        // Review
        usleep(GetRandomTime(200));
        printf("critic finished.\r\n");
        writersCount = 0;
        pthread_cond_broadcast(&critic_cond);
        
        if (pthread_mutex_unlock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the writer mutex.\n");
            exit (-1);
        }
        if (pthread_mutex_unlock(&critic_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the critic mutex.\n");
            exit (-1);
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    srand(100005);

    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];
    pthread_t criticThread;

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
    	    fprintf(stderr,"Couldn't create the reader threads.");
            exit (-1);
        }
    }

    rc = pthread_create(&criticThread,
                    NULL,
                    (void*) Critic,
                    NULL);

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
            fprintf(stderr,"Couldn't create the writer threads.");
            exit (-1);
        }
    }

    if (rc != 0) {
        fprintf(stderr,"Couldn't create the critic thread.");
        exit (-1);
    }

    // At this point, the readers and writers should perform their operations

    // Wait for the Readers
    for (i = 0; i < READERS_COUNT; i++) 
        pthread_join(readerThreads[i],NULL);

    // Wait for the Writers
    for (int i=0;i<WRITERS_COUNT;i++) {
        pthread_join(writerThreads[i],NULL);
    }

    usleep(1000);

    pthread_cancel(criticThread);

    pthread_mutex_destroy(&writer_mutex);
    pthread_mutex_destroy(&readers_mutex);
    pthread_mutex_destroy(&critic_mutex);
    pthread_cond_destroy(&critic_cond);

    return (0);
}