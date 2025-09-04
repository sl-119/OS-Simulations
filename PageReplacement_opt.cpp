//LEKHESH 22CSB0C03
//PAGE REPLACEMENT ALGO WITH OPT

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
        if (table->frames[i] == page) return true;
    }
    return false;
}

// Function to print the current state of the page table
void print_table(PageTable *table) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (table->frames[i] != -1)
            cout << table->frames[i] << " ";
        else
            cout << "- ";
    }
    cout << " || Page Faults: " << table->page_faults << endl;
}

// Function to find the position to replace using Optimal Algorithm
int find_pos_opt(PageTable *table, int pages[], int np, int curr) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (table->frames[i] == -1) return i; // Empty frame found
    }

    int pos[MAX_FRAMES];
    int max_dist = -1, retpos = -1;

    // Initialize positions to a large value (indicating infinite distance)
    for (int i = 0; i < MAX_FRAMES; i++) {
        pos[i] = 1e9;
    }

    // Determine future positions of pages currently in the frame
    for (int i = 0; i < MAX_FRAMES; i++) {
        for (int j = curr + 1; j < np; j++) {
            if (table->frames[i] == pages[j]) {
                pos[i] = j;
                break;
            }
        }
    }

    // Find the page that will not be used for the longest time
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (pos[i] > max_dist) {
            max_dist = pos[i];
            retpos = i;
        }
    }

    return retpos;
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
    for (int i = 0; i < 3; i++) { // Spawning 3 processes
        if (fork() == 0) { // Child process
            for (int j = 0; j < np; j++) {
                semaphore_wait(sem_id); // Lock access

                cout << "Process " << getpid() << " requesting page " << pages[j] << " || ";

                if (!present(table, pages[j])) {
                    int pos = find_pos_opt(table, pages, np, j);
                    table->frames[pos] = pages[j];
                    table->page_faults++;
                    print_table(table);
                } else {
                    print_table(table);
                }

                semaphore_signal(sem_id); // Release access
                usleep(50000); // Sleep to simulate delay
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

/*
/// 

WITHOUT SYSTEM V APPROCH 
//PAGE REPLACEMENT ALGORITHM 
//S LEKHESH
//22CSB0C03

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

int find_pos_opt(int table_frame[],int nf,int pages[],int np,int curr)
{
	for(int i=0;i<nf;i++)
	{
		if(table_frame[i]==-1)
		return i;
	}
	
	int max=-1;
	int pos[nf]={0};
	
	for(int i=0;i<nf;i++)
	{
		pos[i]=1e9;
		
		for(int j=curr;j<np;j++)
		{
			if(table_frame[i]==pages[j])
			{
				pos[i]=j;
				break;
			}
		}
	}
	
	int retpos=-1;
	for(int i=0;i<nf;i++)
	{
		if(max<pos[i])
		{
			max=pos[i];
			retpos=i;
		}
	}
	return retpos;
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
		//for optimal
			int curr=i;
			pos=find_pos_opt(table_frame,nf,pages,np,curr);
			
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
