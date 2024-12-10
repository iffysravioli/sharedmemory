#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_LOOPS 25

void ParentProcess(int[]);
void ChildProcess(int[]);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create shared memory for 2 integers (BankAccount and Turn)
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }
    printf("Shared memory created...\n");

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }
    printf("Shared memory attached...\n");

    // Initialize shared variables
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn (0: Parent's turn, 1: Child's turn)

    printf("Shared memory initialized. BankAccount = %d, Turn = %d\n", ShmPTR[0], ShmPTR[1]);

    // Fork a child process
    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    }
    else if (pid == 0) {
        ChildProcess(ShmPTR); // Child process (Poor Student)
        exit(0);
    } else {
        ParentProcess(ShmPTR); // Parent process (Dear Old Dad)
    }

    // Wait for the child to complete
    wait(&status);

    // Detach and remove shared memory
    shmdt((void *)ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Shared memory detached and removed...\n");
    printf("Program exits...\n");

    return 0;
}

void ParentProcess(int ShmPTR[]) {
    int i; // Declare loop variable outside the for loop
    int account, balance;
    srand(time(NULL));

    for (i = 0; i < NUM_LOOPS; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        account = ShmPTR[0]; // Copy BankAccount to local variable

        // Wait until it's the parent's turn
        while (ShmPTR[1] != 0);

        if (account <= 100) {
            // Try to deposit money
            balance = rand() % 101; // Random amount between 0 and 100
            if (balance % 2 == 0) {
                account += balance; // Deposit money
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Copy the updated account balance back to shared memory
        ShmPTR[0] = account;
        ShmPTR[1] = 1; // Set Turn = 1 (Child's turn)
    }
}

void ChildProcess(int ShmPTR[]) {
    int i; // Declare loop variable outside the for loop
    int account, balance;
    srand(time(NULL) * getpid()); // Different seed for the child process

    for (i = 0; i < NUM_LOOPS; i++) {
        sleep(rand() % 6); // Sleep for 0-5 seconds

        account = ShmPTR[0]; // Copy BankAccount to local variable

        // Wait until it's the child's turn
        while (ShmPTR[1] != 1);

        balance = rand() % 51; // Random amount between 0 and 50
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance; // Withdraw money
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Copy the updated account balance back to shared memory
        ShmPTR[0] = account;
        ShmPTR[1] = 0; // Set Turn = 0 (Parent's turn)
    }
}
