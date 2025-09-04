//LEKHESH 22CSB0C03
//PAGE REPLACEMENT ALGO WITH FIFO


#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>
#include <cstring>
using namespace std;

#define MAX_FRAMES 3

// Structure to store page table in shared memory
struct PageTable {
    int frames[MAX_FRAMES];
    int pos;  // For FIFO replacement
    int page_faults;
};

// Semaphore union for control
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// Function to initialize semaphore
void init_semaphore(int sem_id) {
    union semun sem_union;
    sem_union.val = 1;
    semctl(sem_id, 0, SETVAL, sem_union);
}

// Wait (P operation)
void semaphore_wait(int sem_id) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_op, 1);
}

// Signal (V operation)
void semaphore_signal(int sem_id) {
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_op, 1);
}

// Function to check if a page is present in the page table
bool present(PageTable *table, int page) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (table->frames[i] == page) {
            return true;
        }
    }
    return false;
}

// Function to print the current state of the page table
void print_table(PageTable *table) {
    cout << "Page Table: ";
    for (int i = 0; i < MAX_FRAMES; i++) {
        cout << table->frames[i] << " ";
    }
    cout << " || Page Faults: " << table->page_faults << endl;
}

int main() {
    // Step 1: Create shared memory
    int shm_id = shmget(IPC_PRIVATE, sizeof(PageTable), IPC_CREAT | 0666);
    if (shm_id == -1) {
        cerr << "Error creating shared memory" << endl;
        return 1;
    }

    // Attach shared memory
    PageTable *table = (PageTable *)shmat(shm_id, nullptr, 0);
    if (table == (void *)-1) {
        cerr << "Error attaching shared memory" << endl;
        return 1;
    }

    // Initialize page table
    memset(table->frames, -1, sizeof(table->frames));
    table->pos = 0;
    table->page_faults = 0;

    // Step 2: Create semaphore
    int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        cerr << "Error creating semaphore" << endl;
        return 1;
    }
    init_semaphore(sem_id);

    // Pages to be requested
    int pages[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int np = sizeof(pages) / sizeof(pages[0]);

    // Step 3: Fork multiple processes to simulate page requests
    for (int i = 0; i < 5; i++) {
        if (fork() == 0) { // Child process
            for (int j = 0; j < np; j++) {
                semaphore_wait(sem_id); // Lock access

                cout << "Process " << getpid() << " requesting page " << pages[j] << " || ";

                if (!present(table, pages[j])) {
                    // Page fault, perform FIFO replacement
                    table->frames[table->pos] = pages[j];
                    table->pos = (table->pos + 1) % MAX_FRAMES;
                    table->page_faults++;
                    print_table(table);
                } else {
                    print_table(table);
                }

                semaphore_signal(sem_id); // Release access
                usleep(50000); // Slee  p to simulate delay
            }
            exit(0);
        }
    }

    // Wait for all child processes to complete
    while (wait(nullptr) > 0);

    // Detach and remove shared memory
    shmdt(table);
    shmctl(shm_id, IPC_RMID, nullptr);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}

///

/*
WITHOUT SYSTEM V IPC
#include<iostream>
using namespace std;

bool present(int table_frame[],int nf,int page)
{
	for(int i=0;i<nf;i++)
	{
		if(table_frame[i]==page)
		return true;	
	}	
	return false;
}

void print(int table_frame[],int nf)
{
	for(int i=0;i<nf;i++)
	{
		cout<<table_frame[i]<<" ";
	}
	cout<<"||";
}

int main()
{
	int nf,pos=0;
	cout<<"no of frames\n";
	cin>>nf;
	
	int table_frame[nf];
	
	for(int i=0;i<nf;i++)
	table_frame[i]=-1;
	
	int np;
	cout<<"no of pages\n";
	cin>>np;
	
	int pages[np];
	cout<<"pages\n";
	for(int i=0;i<np;i++)
	{
		cin>>pages[i];
	}
	
	int count=0;
	cout<<"Position of frames after each request\n";
	for(int i=0;i<np;i++)
	{
		cout<<"Page table after request from "<<pages[i]<<" || ";
		
		if(!present(table_frame,nf,pages[i]))
		{			
		//	for fifo 
			
			pos=(pos+1)%nf;
			
			table_frame[pos]=pages[i];
			print(table_frame,nf);
		
			cout<<"page fault\n";
			count++;
			continue;
		}
		
		print(table_frame,nf);
		cout<<"\n";
	}
	
	cout<<"No of page faults : "<<count<<"\n";
}

/*no of frames 3
  no of pages 20 */
  
  
/* pages are
7 0 1 2 0
3 0 4 2 3
0 3 2 1 2
0 1 7 0 1
*/

///
