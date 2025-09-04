// LEKHESH 22CSB0C03
/*
Implement deadlock avoidance, detection algorithms using System V 
IPC facilities.
*/

/*
      Banker's Algorithm
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Global matrices and arrays for resource management
int need[100][100], alloc[100][100], max[100][100], available[100];
bool finish[100]; // This will track whether a process has finished
int sequence[100]; // To store the safe sequence of processes

/*
  Safety Algorithm:

  1) Let Work and Finish be vectors of length ‘m’ and ‘n’ respectively.
     Initialize: Work = Available
     Finish[i] = false; for i=1, 2, 3, 4….n

  2) Find an i such that both
     a) Finish[i] = false
     b) Needi <= Work if no such i exists, goto step (4)

  3) Work = Work + Allocation
     Finish[i] = true
     Go to step (2)

  4) If Finish[i] = true for all i, then the system is in a safe state, otherwise unsafe state
*/

// Function to check if the system is in a safe state
void issafe(int N, int M) {
    int i, j, work[100], count = 0;
    // Initialize Work[] as Available resources
    for (i = 0; i < M; i++)
        work[i] = available[i];

    // Initialize Finish[] as false for all processes
    for (i = 0; i < 100; i++)
        finish[i] = false;

    // Process the safe sequence
    while (count < N) {
        bool canAllot = false;
        for (i = 0; i < N; i++) {
            if (finish[i] == false) {
                // Check if the process's remaining needs can be satisfied with the available resources
                for (j = 0; j < M; j++) {
                    if (work[j] < need[i][j]) {
                        break; // If not, move to next process
                    }
                }
                // If all needs can be met, allocate resources to the process
                if (j == M) {
                    for (j = 0; j < M; j++) {
                        work[j] += alloc[i][j]; // Add the allocated resources to work
                    }
                    sequence[count++] = i; // Record the process in the safe sequence
                    finish[i] = true; // Mark the process as finished
                    canAllot = true;
                }
            }
        }

        // If no process can be allotted, the system is in an unsafe state
        if (canAllot == false) {
            printf("System is not safe\n");
            return;
        }
    }

    // If we reach here, the system is in a safe state
    printf("System is in safe state\n");
    printf("Safe sequence: ");
    for (i = 0; i < N; i++)
        printf("%d ", sequence[i]);
    printf("\n");
}

int main() {
    int n, m, i, j;

    // Step 1: Input number of processes and resources
    printf("Enter number of processes: ");
    scanf("%d", &n);

    printf("Enter number of resources: ");
    scanf("%d", &m);

    // Step 2: Input available resources
    printf("Enter the available resources:\n");
    for (i = 0; i < m; i++)
        scanf("%d", &available[i]);

    // Step 3: Input Allocation matrix
    printf("Enter the Allocation Matrix:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            scanf("%d", &alloc[i][j]);
        }
    }

    // Step 4: Input Max demand matrix
    printf("Enter the matrix for maximum demand of each process:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            scanf("%d", &max[i][j]);
        }
    }

    // Step 5: Calculate the Need matrix (Need = Max - Allocation)
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }

    // Step 6: Check if the system is in a safe state
    issafe(n, m);

    // Step 7: Handle resource request from a process
    int indx, arr[100];
    printf("Enter the process no for resource request: ");
    scanf("%d", &indx);

    printf("Enter the requested instances of each resource: ");
    for (i = 0; i < m; i++)
        scanf("%d", &arr[i]);

    /*
       RESOURCE-REQUEST ALGORITHM

       1) If Requesti <= Needi
          Go to step (2); otherwise, raise an error condition since the process has exceeded its maximum claim.

       2) If Requesti <= Available, go to step (3); otherwise, Pi must wait since the resources are not available.

       3) Pretend to allocate the requested resources to process Pi by modifying the state as follows:
          Available = Available - Requesti
          Allocationi = Allocationi + Requesti
          Needi = Needi - Requesti
    */

    // Step 8: Check if the resource request can be granted
    for (i = 0; i < m; i++) {
        if (need[indx][i] < arr[i] || arr[i] > available[i]) {
            printf("Cannot request resources\n");
            break;
        }
    }

    // Step 9: If the request is valid, update the system state and check for safety
    if (i == m) {
        for (i = 0; i < m; i++) {
            alloc[indx][i] += arr[i]; // Allocate requested resources
            available[i] -= arr[i];   // Update available resources
            need[indx][i] -= arr[i];  // Update the need matrix
        }

        // Step 10: Check if the system is still in a safe state after allocation
        issafe(n, m);
    }

    return 0;
}
