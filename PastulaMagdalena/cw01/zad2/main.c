#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define WRITER_TURNS        10
#define READER_TURNS        10
#define READERS_COUNT       10
#define WRITERS_COUNT       5

#define BUFFER_MAX_SIZE     100

int readersCount;

int buffer_size;
bool is_buffer_full;


pthread_mutex_t readers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_write_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_read_cond = PTHREAD_COND_INITIALIZER;

uint32_t GetRandomTime(uint32_t max_val) {
    return rand()%max_val + 1;
}


int Writer(void* data) {
    int threadId = *(int*) data;
    
    for (int i = 0; i < WRITER_TURNS; i++) {
        if (pthread_mutex_lock(&buffer_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the buffer mutex.\n");
            exit (-1);
        }
        while (is_buffer_full) {
            printf("(W) Writer %d waiting to write to buffer.\r\n", threadId);
            pthread_cond_wait(&buffer_write_cond, &buffer_mutex);
        }
        //Write
        printf("(W) Writer %d started writing to buffer...", threadId);
        fflush(stdout);
        buffer_size += GetRandomTime(800);
        is_buffer_full = (buffer_size >= BUFFER_MAX_SIZE);
        printf("writer finished\n");
            
        if (pthread_mutex_unlock(&buffer_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the buffer mutex.\n");
            exit (-1);
        }
        // Think, think, think, think
        usleep(GetRandomTime(1000));
    }

    pthread_cond_broadcast(&buffer_read_cond);
    free(data);
    return 0;
}


int SuperiorWriter(void* data) {
    while (1) {
        if (pthread_mutex_lock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the writer mutex.\n");
            exit (-1);
        }

        while (!is_buffer_full) {
            printf("(B) Buffer waiting for data to write to file\r\n");
            pthread_cond_wait(&buffer_read_cond, &writer_mutex);
        }

        if (pthread_mutex_lock(&buffer_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the writer mutex.\n");
            exit (-1);
        }
        printf("(B) Buffer started writing to file...");
        buffer_size = 0;
        is_buffer_full = false;
        printf("buffer finished\r\n");
        pthread_cond_broadcast(&buffer_write_cond);
        pthread_mutex_unlock(&buffer_mutex);
        
        if (pthread_mutex_unlock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the writer mutex.\n");
            exit (-1);
        }

        usleep(GetRandomTime(1000));
    }
}


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

void createThread(pthread_t* handle, int (*handler)(void*), void* arg) {
    int rc = pthread_create(handle,
                            NULL,
                            (void*)handler,
                            arg);
    
    if (rc != 0) {
        fprintf(stderr,"Couldn't create thread");
        exit (-1);
    }
}

int main(int argc, char* argv[])
{
    srand(100005);

    pthread_t writerThreads[WRITERS_COUNT];
    pthread_t readerThreads[READERS_COUNT];
    pthread_t superiorWriterThread;

    
    for (int i=0;i<WRITERS_COUNT;i++) {
        usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;
        createThread(&writerThreads[i], Writer, threadId);
    }

    createThread(&superiorWriterThread, SuperiorWriter, NULL);

    // Create the Reader threads
    for (int i = 0; i < READERS_COUNT; i++) {
	// Reader initialization - takes random amount of time
	    usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;

        createThread(&readerThreads[i], Reader, threadId);
    }

    // At this point, the readers and writers should perform their operations
    // Wait for the Readers
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readerThreads[i],NULL);
    }
    
    for (int i=0;i<WRITERS_COUNT;i++) {
        pthread_join(writerThreads[i],NULL);
    }

    pthread_cancel(superiorWriterThread);

    return (0);
}