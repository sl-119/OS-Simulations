//LEKHESH 22CSB0C03

/* Create your own thread library, which has the features of pthread 
library by using appropriate system calls (UContext related calls). 
Containing functionality for creation, termination of threads with 
simple round robin scheduling algorithm and synchronization 
features.*/

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_THREADS 10 // Max number of threads
#define STACK_SIZE 8192 // Size of stack for each thread

// Define structure to hold thread information
typedef struct thread {
    ucontext_t context;   // Thread context
    void (*function)(void); // Function to be executed by the thread
    bool is_done;         // Flag to check if the thread is finished
} thread_t;

// Global variables
thread_t threads[MAX_THREADS];
int thread_count = 0;         // Current number of threads
int current_thread = -1;      // Index of the current running thread
ucontext_t main_context;      // Main context to return when a thread terminates

// Mutex structure
typedef struct {
    int locked;  // 0 - unlocked, 1 - locked
} mutex_t;

// Mutex initialization
void mutex_init(mutex_t* m) {
    m->locked = 0;
}

// Mutex lock
void mutex_lock(mutex_t* m) {
    while (__sync_lock_test_and_set(&m->locked, 1)) {
        // Busy wait (spinlock)
        usleep(10);
    }
}

// Mutex unlock
void mutex_unlock(mutex_t* m) {
    __sync_lock_release(&m->locked);
}

// Simple round-robin scheduler
void scheduler() {
    int next_thread = (current_thread + 1) % thread_count;
    if (next_thread != current_thread) {
        current_thread = next_thread;
        swapcontext(&threads[current_thread].context, &main_context); // Switch to the next thread
    }
}

// Thread function to simulate the execution of threads
void thread_func_wrapper(int thread_id) {
    threads[thread_id].function();  // Execute the thread function
    threads[thread_id].is_done = true; // Mark thread as done
    scheduler();  // Schedule the next thread
}

// Custom thread create function
int thread_create(void (*function)(void)) {
    if (thread_count >= MAX_THREADS) {
        printf("Maximum thread limit reached\n");
        return -1;  // Max thread limit reached
    }

    int thread_id = thread_count++;
    threads[thread_id].function = function;
    threads[thread_id].is_done = false;

    // Allocate stack memory for the thread
    char* stack = (char*)malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc failed for thread stack");
        return -1;
    }

    // Initialize thread context
    getcontext(&threads[thread_id].context);
    threads[thread_id].context.uc_stack.ss_sp = stack;
    threads[thread_id].context.uc_stack.ss_size = STACK_SIZE;
    threads[thread_id].context.uc_link = &main_context;  // Link to main context after completion

    // Create the thread by setting the function to be executed
    makecontext(&threads[thread_id].context, (void(*)()) thread_func_wrapper, 1, thread_id);

    return thread_id; // Return thread ID
}

// Thread termination (return to main context)
void thread_exit() {
    setcontext(&main_context);  // Return to main context (finish thread execution)
}

// Initialize and start the thread scheduler
void thread_start() {
    while (true) {
        bool all_done = true;
        for (int i = 0; i < thread_count; i++) {
            if (!threads[i].is_done) {
                all_done = false;
                current_thread = i;  // Set the current thread to the i-th thread
                swapcontext(&main_context, &threads[i].context);  // Switch to the thread
            }
        }
        if (all_done) break; // All threads are done, exit the scheduler
    }
}

// Example thread function
void my_thread() {
    printf("Thread %d started\n", current_thread);
    sleep(1);
    printf("Thread %d finished\n", current_thread);
    thread_exit(); // Exit this thread
}

// Main function to test the custom thread library
int main() {
    // Create threads
    for (int i = 0; i < 5; i++) {
        thread_create(my_thread); // Create 5 threads
    }

    // Start the custom thread scheduler
    thread_start();

    printf("All threads finished.\n");

    return 0;
}
