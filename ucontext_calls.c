//LEKHESH 22CSB0C03
//ALL 4 COOROUTINES IMPLEMENTATION IN ONE CODE
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep

ucontext_t context1, context2, main_context;

void function1() {
    for (int i = 0; i < 3; i++) {
        printf("Function 1, iteration %d - Entering\n", i + 1);
        printf("Function 1: Switching to Function 2\n");
        swapcontext(&context1, &context2); // Switch to function2
        printf("Function 1, iteration %d - Resumed\n", i + 1); // After returning from function2
        sleep(1); // Added delay for clearer observation
    }
    printf("Function 1: Finished. Returning to main.\n");
    setcontext(&main_context); // Return to main context directly
}

void function2() {
    for (int i = 0; i < 3; i++) {
        printf("Function 2, iteration %d - Entering\n", i + 1);
        printf("Function 2: Switching to Function 1\n");
        swapcontext(&context2, &context1); // Switch to function1
        printf("Function 2, iteration %d - Resumed\n", i + 1); // After returning from function1
        sleep(1); // Added delay for clearer observation
    }
    printf("Function 2: Finished. Returning to main.\n");
    setcontext(&main_context); // Return to main context directly
}

int main() {
    // Allocate stack memory for each context
    char stack1[8192];
    char stack2[8192];

    // Initialize context1 for function1
    getcontext(&context1);
    context1.uc_stack.ss_sp = stack1;
    context1.uc_stack.ss_size = sizeof(stack1);
    context1.uc_link = &main_context;  // Return to main after completion
    makecontext(&context1, function1, 0); // Link to function1

    // Initialize context2 for function2
    getcontext(&context2);
    context2.uc_stack.ss_sp = stack2;
    context2.uc_stack.ss_size = sizeof(stack2);
    context2.uc_link = &main_context;  // Return to main after completion
    makecontext(&context2, function2, 0); // Link to function2

    // Start execution using setcontext() instead of swapcontext()
    printf("Main: Starting context1 using setcontext()\n");
    setcontext(&context1);

    // This line will not be reached as setcontext() does not return
    printf("Main context resumes after functions finish.\n");

    return 0;
}
