//LEKHESH 22CSB0C03
//PRODUCER_CONSUMER PROBLEM


#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>

#define BUFFER_SIZE 5
#define PRODUCER_COUNT 10
#define CONSUMER_COUNT 10

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main() {
    int shmid, semid;
    int *buffer;
    int in = 0, out = 0;

    // Create shared memory
    shmid = shmget(IPC_PRIVATE, BUFFER_SIZE * sizeof(int), IPC_CREAT | 0666);
    buffer = (int *)shmat(shmid, NULL, 0);

    // Create semaphores
    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
    union semun sem_union;

    // Initialize semaphores
    sem_union.val = 1; // mutex
    semctl(semid, 0, SETVAL, sem_union);
    
    sem_union.val = BUFFER_SIZE; // empty slots
    semctl(semid, 1, SETVAL, sem_union);
    
    sem_union.val = 0; // full slots
    semctl(semid, 2, SETVAL, sem_union);

    // Semaphore operations for P (wait) and V (signal)
    struct sembuf P_op = {0, -1, 0}; // Decrease
    struct sembuf V_op = {0, 1, 0};  // Increase

    // Fork a producer and a consumer process
    if (fork() == 0) {
        // Producer process
        for (int i = 0; i < PRODUCER_COUNT; i++) {
            P_op.sem_num = 1; // empty
            semop(semid, &P_op, 1); // Wait for empty slot
            
            P_op.sem_num = 0; // mutex
            semop(semid, &P_op, 1); // Enter critical section
            
            buffer[in] = i;
            printf("Produced: %d\n", buffer[in]);
            in = (in + 1) % BUFFER_SIZE;
            
            V_op.sem_num = 0; // mutex
            semop(semid, &V_op, 1); // Leave critical section
            
            V_op.sem_num = 2; // full
            semop(semid, &V_op, 1); // Signal full slot
            
            sleep(1);
        }
        exit(0);
    } else if (fork() == 0) {
        // Consumer process
        for (int i = 0; i < CONSUMER_COUNT; i++) {
            P_op.sem_num = 2; // full
            semop(semid, &P_op, 1); // Wait for full slot
            
            P_op.sem_num = 0; // mutex
            semop(semid, &P_op, 1); // Enter critical section
            
            int item = buffer[out];
            printf("Consumed: %d\n", item);
            out = (out + 1) % BUFFER_SIZE;
            
            V_op.sem_num = 0; // mutex
            semop(semid, &V_op, 1); // Leave critical section
            
            V_op.sem_num = 1; // empty
            semop(semid, &V_op, 1); // Signal empty slot
            
            sleep(1);
        }
        exit(0);
    } else {
        // Parent process waits for children to finish
        wait(NULL);
        wait(NULL);

        // Clean up
        shmdt(buffer);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, sem_union);
    }
    return 0;
}
