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
bool is_buffer_empty = true;

bool is_finished;

pthread_mutex_t reader_mutex = PTHREAD_MUTEX_INITIALIZER;
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
        if (pthread_mutex_lock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the writer mutex.\n");
            exit (-1);
        }
        if (!is_finished) {
            while (!is_buffer_empty) {
                printf("(W) Writer %d waiting to write to buffer.\r\n", threadId);
                pthread_cond_wait(&buffer_write_cond, &writer_mutex);
            }
            if (pthread_mutex_lock(&buffer_mutex) != 0) {
                fprintf(stderr, "Error occured during locking the buffer mutex.\n");
                exit (-1);
            }

            //Write
            printf("(W) Writer %d started writing to buffer...", threadId);
            fflush(stdout);
            buffer_size += GetRandomTime(800);
            // buffer_size += 5;
            is_buffer_full = (buffer_size >= BUFFER_MAX_SIZE);
            printf("writer finished\n");    
        
                
            if (is_buffer_full) {
                is_buffer_empty = false;
                printf("(W) Buffer is full.\r\n");
                pthread_cond_broadcast(&buffer_read_cond);
            }
            if (is_finished) {
                is_buffer_empty = true;
            }

            if (threadId == WRITERS_COUNT-1 && i == WRITER_TURNS-1) {
                is_finished = true;
                is_buffer_full = true;
                pthread_cond_broadcast(&buffer_read_cond);
                printf("Program finished. Nothing more to write.\r\n");
            }

            if (pthread_mutex_unlock(&buffer_mutex) != 0) {
                fprintf(stderr, "Error occured during unlocking the buffer mutex.\n");
                exit (-1);
            }
        }
        if (pthread_mutex_unlock(&writer_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the writer mutex.\n");
            exit (-1);
        }
        // Think, think, think, think
        usleep(GetRandomTime(1000));
    }

    free(data);
    return 0;
}


int Reader(void* data) {
    int threadId = *(int*) data;
    
    for (int i = 0; i < READER_TURNS; i++) {
        if (pthread_mutex_lock(&reader_mutex) != 0) {
            fprintf(stderr, "Error occured during locking the reader mutex.\n");
            exit (-1);
        } 
        if (!is_finished) {
            while (!is_buffer_full) {
                printf("(R) Reader %d waiting to read buffer.\r\n", threadId);
                pthread_cond_wait(&buffer_read_cond, &reader_mutex);
            }
            if (pthread_mutex_lock(&buffer_mutex) != 0) {
                fprintf(stderr, "Error occured during locking the buffer mutex.\n");
                exit (-1);
            }

            // Read
            printf("(R) Reader %d started reading...", threadId);        
            fflush(stdout);
            // Read, read, read
            buffer_size -= GetRandomTime(700);
            // buffer_size -= 5;
            if (buffer_size < 0) buffer_size = 0;
            is_buffer_empty = (buffer_size <= 0);
            printf("reader finished\n");
        
            
            if (is_buffer_empty) {
                is_buffer_full = false;
                printf("(R) Buffer is empty.\r\n");
                pthread_cond_broadcast(&buffer_write_cond);
            }
            
            if (is_finished) {
                is_buffer_full = true;
            }

            if (threadId == READERS_COUNT-1 && i == READER_TURNS-1) {
                printf("Program finished. Nothing more to read.\r\n");
                is_finished = true;
                is_buffer_empty = true;
                pthread_cond_broadcast(&buffer_write_cond);
            }

            // Release ownership of the writing semaphore
            if (pthread_mutex_unlock(&buffer_mutex) != 0) {
                fprintf(stderr, "Error occured during unlocking the buffer mutex.\n");
                exit (-1);
            }
        }

        if (pthread_mutex_unlock(&reader_mutex) != 0) {
            fprintf(stderr, "Error occured during unlocking the reader mutex.\n");
            exit (-1);
        }
        
        usleep(GetRandomTime(800));
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
    
    for (int i=0;i<WRITERS_COUNT;i++) {
        usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;
        createThread(&writerThreads[i], Writer, threadId);
    }

    // Create the Reader threads
    for (int i = 0; i < READERS_COUNT; i++) {
	    usleep(GetRandomTime(1000));
        int* threadId = malloc(sizeof(int));
        *threadId = i;

        createThread(&readerThreads[i], Reader, threadId);
    }

    // At this point, the readers and writers should perform their operations
    // Wait for the Readers
    for (int i=0;i < WRITERS_COUNT;i++) {
        pthread_join(writerThreads[i],NULL);
    }

    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readerThreads[i],NULL);
    }

    return (0);
}