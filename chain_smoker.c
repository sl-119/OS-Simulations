//LEKHESH 22CSB0C03
//CHAIN SMOKER PROBLEM


#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t agentSem;  // Controls when the agent places items on the table
sem_t tobaccoSem, paperSem, matchSem;  // Signals each smoker type
sem_t tobacco, paper, match;  // Available ingredients on the table

void* agent(void* arg) {
    while (1) {
        sem_wait(&agentSem);  // Wait for smokers to finish smoking

        int r = rand() % 3;  // Randomly pick which two items to place
        if (r == 0) {
            printf("Agent places tobacco and paper\n");
            sem_post(&matchSem);  // Signal the smoker with matches
        } else if (r == 1) {
            printf("Agent places paper and matches\n");
            sem_post(&tobaccoSem);  // Signal the smoker with tobacco
        } else {
            printf("Agent places tobacco and matches\n");
            sem_post(&paperSem);  // Signal the smoker with paper
        }
    }
    return NULL;
}

// Each smoker's routine
void* smoker_with_tobacco(void* arg) {
    while (1) {
        sem_wait(&tobaccoSem);  // Wait for agent to put paper and matches
        printf("Smoker with tobacco makes a cigarette\n");
        sleep(1);  // Time to "smoke"
        printf("Smoker with tobacco is smoking\n");
        sem_post(&agentSem);  // Signal the agent
    }
    return NULL;
}

void* smoker_with_paper(void* arg) {
    while (1) {
        sem_wait(&paperSem);  // Wait for agent to put tobacco and matches
        printf("Smoker with paper makes a cigarette\n");
        sleep(1);  // Time to "smoke"
        printf("Smoker with paper is smoking\n");
        sem_post(&agentSem);  // Signal the agent
    }
    return NULL;
}

void* smoker_with_matches(void* arg) {
    while (1) {
        sem_wait(&matchSem);  // Wait for agent to put tobacco and paper
        printf("Smoker with matches makes a cigarette\n");
        sleep(1);  // Time to "smoke"
        printf("Smoker with matches is smoking\n");
        sem_post(&agentSem);  // Signal the agent
    }
    return NULL;
}

int main() {
    pthread_t agentThread, tobaccoSmoker, paperSmoker, matchSmoker;

    // Initialize semaphores
    sem_init(&agentSem, 0, 1);  // Start with agent ready to place items
    sem_init(&tobaccoSem, 0, 0);
    sem_init(&paperSem, 0, 0);
    sem_init(&matchSem, 0, 0);

    // Create threads for agent and smokers
    pthread_create(&agentThread, NULL, agent, NULL);
    pthread_create(&tobaccoSmoker, NULL, smoker_with_tobacco, NULL);
    pthread_create(&paperSmoker, NULL, smoker_with_paper, NULL);
    pthread_create(&matchSmoker, NULL, smoker_with_matches, NULL);

    // Wait for threads to finish (they won’t in this infinite loop)
    pthread_join(agentThread, NULL);
    pthread_join(tobaccoSmoker, NULL);
    pthread_join(paperSmoker, NULL);
    pthread_join(matchSmoker, NULL);

    // Destroy semaphores
    sem_destroy(&agentSem);
    sem_destroy(&tobaccoSem);
    sem_destroy(&paperSem);
    sem_destroy(&matchSem);

    return 0;
}

