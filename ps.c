#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_BUF 1024

// Structure to hold process information
typedef struct {
    pid_t pid;
    char name[MAX_BUF];
    char state;
    uid_t uid;
    size_t rss;
    char tty[MAX_BUF];
    unsigned long utime, stime; // user time and system time
} Process;

// Function to get process name
void get_name(pid_t pid, char* name) {
    char path[MAX_BUF];
    FILE* file;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    fgets(name, MAX_BUF, file);
    name[strcspn(name, "\n")] = '\0';  // Remove newline

    fclose(file);
}

// Function to get process info
void get_process_info(pid_t pid, Process* process) {
    char path[MAX_BUF];
    FILE* file;
    char line[MAX_BUF];

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "State:", 6) == 0) {
            process->state = line[7];
        } else if (strncmp(line, "Uid:", 4) == 0) {
            process->uid = atoi(line + 5);
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            process->rss = atoi(line + 7);
        }
    }

    fclose(file);

    // Get TTY information
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file != NULL) {
        fscanf(file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
               &process->utime, &process->stime);
        fclose(file);

        // Get the TTY associated with the process
        snprintf(path, sizeof(path), "/proc/%d/fd/0", pid);
        char tty_path[MAX_BUF];
        ssize_t len = readlink(path, tty_path, sizeof(tty_path) - 1);
        if (len != -1) {
            tty_path[len] = '\0';  // Null-terminate the string
            strcpy(process->tty, tty_path);
        } else {
            strcpy(process->tty, "?");
        }
    }
}

// Function to print process info
void print_process_info(Process* process) {
    printf("%-6d %-10s %-5.2f %s\n",
           process->pid, process->tty, (process->utime + process->stime) / (double)sysconf(_SC_CLK_TCK),
           process->name);
}

// Function to list processes for a specific user
void list_user_processes(const char* username) {
    struct passwd* pw = getpwnam(username);
    if (!pw) {
        fprintf(stderr, "User not found: %s\n", username);
        return;
    }

    DIR* dir;
    struct dirent* ent;
    Process process;

    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }

    printf("  PID TTY          TIME CMD\n");

    while ((ent = readdir(dir)) != NULL) {
        // Check if entry is a process directory
        if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
            process.pid = atoi(ent->d_name);
            get_name(process.pid, process.name);
            get_process_info(process.pid, &process);

            // Check if the UID matches the user's UID
            if (process.uid == pw->pw_uid) {
                print_process_info(&process);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // Default: List processes for the current terminal
        char* tty = ttyname(STDIN_FILENO);
        if (tty == NULL) {
            perror("ttyname");
            return 1;
        }

        DIR* dir;
        struct dirent* ent;
        Process process;

        dir = opendir("/proc");
        if (dir == NULL) {
            perror("opendir");
            return 1;
        }

        printf("  PID TTY          TIME CMD\n");

        while ((ent = readdir(dir)) != NULL) {
            // Check if entry is a process directory
            if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
                process.pid = atoi(ent->d_name);
                get_name(process.pid, process.name);
                get_process_info(process.pid, &process);

                // Filter processes by current tty
                if (strstr(process.tty, tty)) {
                    print_process_info(&process);
                }
            }
        }

        closedir(dir);
    } else if (argc == 2) {
        if (strcmp(argv[1], "-a") == 0) {
            // List all processes
            DIR* dir;
            struct dirent* ent;
            Process process;

            dir = opendir("/proc");
            if (dir == NULL) {
                perror("opendir");
                return 1;
            }

            printf("  PID TTY          TIME CMD\n");

            while ((ent = readdir(dir)) != NULL) {
                // Check if entry is a process directory
                if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
                    process.pid = atoi(ent->d_name);
                    get_name(process.pid, process.name);
                    get_process_info(process.pid, &process);

                    print_process_info(&process);
                }
            }

            closedir(dir);
        } else if (strcmp(argv[1], "-ae") == 0) {
            // List all processes with extended info (same as -a for now)
            DIR* dir;
            struct dirent* ent;
            Process process;

            dir = opendir("/proc");
            if (dir == NULL) {
                perror("opendir");
                return 1;
            }

            printf("  PID TTY          TIME CMD\n");

            while ((ent = readdir(dir)) != NULL) {
                // Check if entry is a process directory
                if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
                    process.pid = atoi(ent->d_name);
                    get_name(process.pid, process.name);
                    get_process_info(process.pid, &process);

                    print_process_info(&process);
                }
            }

            closedir(dir);
        } else if (strcmp(argv[1], "-u") == 0) {
            fprintf(stderr, "Usage: ps -u <username>\n");
            return 1;
        } else {
            fprintf(stderr, "Invalid option: %s\n", argv[1]);
            return 1;
        }
    } else if (argc == 3 && strcmp(argv[1], "-u") == 0) {
        // List processes for a specific user
        list_user_processes(argv[2]);
    } else {
        fprintf(stderr, "Usage: ps [-a | -ae | -u <username>]\n");
        return 1;
    }

    return 0;
}

