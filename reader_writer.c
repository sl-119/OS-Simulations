//LEKHESH 22CSB0C03
//READER_WRITER PROBLEM


#include<stdio.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#define N 5

sem_t db, mutex;
int data = 0, readCount = 0;

void init_Sem() {
    sem_init(&db, 0, 1);
    sem_init(&mutex, 0, 1);
}

void *writer(void *arg) {
    int id = *(int *)arg;  // Dereference arg as an integer
    free(arg);              // Free allocated memory after use

    sem_wait(&db);
    printf("writer(%d) is %d\n", id, ++data);
    sem_post(&db);  
    sleep(1);
    return NULL;
}

void *reader(void *arg) {
    int id = *(int *)arg;  // Dereference arg as an integer
    free(arg);              // Free allocated memory after use

    sem_wait(&mutex);
    readCount++;
    if (readCount == 1)
        sem_wait(&db);     // First reader locks the database
    sem_post(&mutex);
    
    printf("reader(%d) is %d\n", id, data);
//    sleep(1);
    
    sem_wait(&mutex);
    readCount--;
    if (readCount == 0)
        sem_post(&db);     // Last reader unlocks the database
    sem_post(&mutex);
    sleep(1);
    return NULL;
}

int main() {
    init_Sem();
    
    pthread_t read[N], write[N];
    
    for (int i = 0; i < N; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&write[i], NULL, writer, arg);
        
        arg = malloc(sizeof(*arg));   // Allocate new memory for each reader thread
        *arg = i;
        pthread_create(&read[i], NULL, reader, arg);
    }
    
    for (int i = 0; i < N; i++) {
        pthread_join(write[i], NULL);
        pthread_join(read[i], NULL);
    }
    return 0;
}

