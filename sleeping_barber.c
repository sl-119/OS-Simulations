// LEKHESH 22CSB0C03
// Sleeping Barber Problem using semaphores and pthreads

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_CHAIRS 3 // Number of waiting chairs
#define NUM_CUSTOMERS 10 // Number of customers

sem_t barber_ready;        // Semaphore for barber being ready
sem_t customer_ready;      // Semaphore for customer being ready
pthread_mutex_t mutex;     // Mutex to protect shared resources

int waiting_customers = 0; // Count of customers waiting

// Function for the barber
void* barber(void* arg) {
    while (1) {
        // Wait for a customer to be ready
        sem_wait(&customer_ready);

        // Barber is ready to cut hair
        pthread_mutex_lock(&mutex);
        waiting_customers--; // A customer is being attended
        printf("Barber is cutting hair. Waiting customers: %d\n", waiting_customers);
        pthread_mutex_unlock(&mutex);

        // Signal the barber is ready
        sem_post(&barber_ready);

        // Simulate hair cutting
        sleep(2);
        printf("Barber finished cutting hair.\n");
    }
}

// Function for a customer
void* customer(void* num) {
    int id = *(int*)num;

    pthread_mutex_lock(&mutex);
    if (waiting_customers < NUM_CHAIRS) {
        // There are empty chairs, customer will wait
        waiting_customers++;
        printf("Customer %d is waiting. Waiting customers: %d\n", id, waiting_customers);

        // Signal that a customer is ready
        sem_post(&customer_ready);
        pthread_mutex_unlock(&mutex);

        // Wait for the barber to be ready
        sem_wait(&barber_ready);

        // Getting a haircut
        printf("Customer %d is getting a haircut.\n", id);
    } else {
        // No empty chairs, customer leaves
        printf("Customer %d left because no chairs are available.\n", id);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t barber_thread;
    pthread_t customer_threads[NUM_CUSTOMERS];
    int customer_ids[NUM_CUSTOMERS];

    // Initialize semaphores and mutex
    sem_init(&barber_ready, 0, 0);
    sem_init(&customer_ready, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Create the barber thread
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Create customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_threads[i], NULL, customer, &customer_ids[i]);
        sleep(1); // Stagger customer arrivals
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Terminate the barber thread (optional, not reachable here)
    pthread_cancel(barber_thread);

    // Clean up semaphores and mutex
    sem_destroy(&barber_ready);
    sem_destroy(&customer_ready);
    pthread_mutex_destroy(&mutex);

    return 0;
}
