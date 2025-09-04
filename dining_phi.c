//LEKHESH 22CSB0C03
//Dining philosopher problem using semaphore and pthreads
  
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define N 5
#define THINKING 2
#define HUNGRY 1
#define EATING 0
#define LEFT (phnum + 4) % N
#define RIGHT (phnum + 1) % N

int state[N];
int phil[N] = { 0, 1, 2, 3, 4 };
sem_t S[N];  // semaphore for each philosopher

void test(int phnum) {
    if (state[phnum] == HUNGRY
        && state[LEFT] != EATING
        && state[RIGHT] != EATING) {
        
        // state that eating
        state[phnum] = EATING;
        sleep(2); // simulate eating time

        printf("Philosopher %d takes fork %d and %d\n",
               phnum + 1, LEFT + 1, phnum + 1);
        printf("Philosopher %d is Eating\n", phnum + 1);

        sem_post(&S[phnum]);  // signal that philosopher can start eating
    }
}

// take up chopsticks
void take_fork(int phnum) {
    state[phnum] = HUNGRY;
    printf("Philosopher %d is Hungry\n", phnum + 1);

    // check if philosopher can eat
    test(phnum);

    // wait if unable to eat
    sem_wait(&S[phnum]);
    sleep(1);  // small delay to simulate time between actions
}

// put down chopsticks
void put_fork(int phnum) {
    state[phnum] = THINKING;
    printf("Philosopher %d putting fork %d and %d down\n",
           phnum + 1, LEFT + 1, phnum + 1);
    printf("Philosopher %d is thinking\n", phnum + 1);

    // test if neighbors can eat
    test(LEFT);
    test(RIGHT);
}

void* philosopher(void* num) {
    int* i = num;

    while (1) {
        sleep(1);  // simulate thinking
        take_fork(*i);
        sleep(1);  // simulate eating
        put_fork(*i);
    }
}

int main() {
    int i;
    pthread_t thread_id[N];

    // initialize the semaphores
    for (i = 0; i < N; i++)
        sem_init(&S[i], 0, 0);

    for (i = 0; i < N; i++) {
        // create philosopher processes
        pthread_create(&thread_id[i], NULL, philosopher, &phil[i]);
        printf("Philosopher %d is thinking\n", i + 1);
    }

    for (i = 0; i < N; i++)
        pthread_join(thread_id[i], NULL);
    
    return 0;
}

