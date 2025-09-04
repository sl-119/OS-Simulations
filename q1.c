#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h> // For unlink and access
#include <signal.h> // For kill and signals
#include <ctype.h>  // For isdigit

// Function to print file details in various formats
void display_file_info(struct stat *fileStat, char *fileName) {
    printf("%c", (fileStat->st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (fileStat->st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (fileStat->st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (fileStat->st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (fileStat->st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (fileStat->st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (fileStat->st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (fileStat->st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c ", (fileStat->st_mode & S_IXOTH) ? 'x' : '-');
    printf("%lu ", fileStat->st_nlink);
    printf("%s ", getpwuid(fileStat->st_uid)->pw_name);
    printf("%s ", getgrgid(fileStat->st_gid)->gr_name);
    printf("%lld ", (long long) fileStat->st_size);
    printf("%.12s ", ctime(&fileStat->st_mtime) + 4);
    printf("%s\n", fileName);
}

// Function to list directory contents with various options
void list_directory(const char *dirName, int longFormat, int showAll, int recursive) {
    DIR *directory;
    struct dirent *entry;
    struct stat fileStat;
    char filePath[1024]; // Increased buffer size

    if ((directory = opendir(dirName)) == NULL) {
        perror("opendir");
        return;
    }

    if (recursive) {
        printf("%s:\n", dirName);
    }

    while ((entry = readdir(directory)) != NULL) {
        if (!showAll && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0))
            continue;

        snprintf(filePath, sizeof(filePath), "%s/%s", dirName, entry->d_name); // Safe snprintf
        if (stat(filePath, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        if (longFormat) {
            display_file_info(&fileStat, entry->d_name);
        } else {
            printf("%s\n", entry->d_name);
        }

        if (S_ISDIR(fileStat.st_mode) && recursive) {
            list_directory(filePath, longFormat, showAll, recursive);
        }
    }

    closedir(directory);
}

// Function to remove files
void delete_files(int argc, char *argv[]) {
    int interactive = 0;

    // Check for interactive option
    if (argc > 2 && strcmp(argv[1], "-i") == 0) {
        interactive = 1;
        argv++;
        argc--;
    }

    for (int i = 1; i < argc; i++) {
        if (access(argv[i], F_OK) == -1) {
            perror("access");
            continue;
        }

        // Ask for confirmation if in interactive mode
        if (interactive) {
            printf("Remove %s? (y/n): ", argv[i]);
            char response;
            scanf(" %c", &response);
            if (response != 'y' && response != 'Y') {
                printf("Skipped: %s\n", argv[i]);
                continue;
            }
        }

        if (unlink(argv[i]) == -1) {
            perror("rm");
        } else {
            printf("Removed: %s\n", argv[i]);
        }
    }
}

// Function to concatenate and display file contents
void concatenate_files(int argc, char *argv[]) {
    FILE *file;
    char buffer[1024];

    for (int i = 1; i < argc; i++) {
        file = fopen(argv[i], "r");
        if (file == NULL) {
            perror("cat");
            continue;
        }

        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }

        fclose(file);
    }
}

// Function to send signals to processes
void send_signals(int argc, char *argv[]) {
    int signalNum = SIGTERM; // Default signal
    int start_index = 1;

    // Check for signal option
    if (argc > 2 && argv[1][0] == '-' && argv[1][1] != '\0') {
        signalNum = atoi(argv[1] + 1); // Convert signal number from string
        start_index = 2; // Start after the signal option
    }

    for (int i = start_index; i < argc; i++) {
        pid_t pid = (pid_t) atoi(argv[i]); // Convert PID from string
        if (kill(pid, signalNum) == -1) {
            perror("kill");
        } else {
            printf("Sent signal %d to PID %d\n", signalNum, pid);
        }
    }
}

// Function to count lines, words, and characters in files
void count_words(int argc, char *argv[]) {
    int countLines = 0, countWords = 0, countChars = 0;
    int countLinesFlag = 0, countWordsFlag = 0, countCharsFlag = 0;

    // Check for flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            countLinesFlag = 1;
        } else if (strcmp(argv[i], "-w") == 0) {
            countWordsFlag = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            countCharsFlag = 1;
        } else {
            FILE *file = fopen(argv[i], "r");
            if (file) {
                char ch;
                int in_word = 0;

                while ((ch = fgetc(file)) != EOF) {
                    countChars++;
                    if (ch == '\n') {
                        countLines++;
                    }
                    if (isspace(ch)) {
                        in_word = 0;
                    } else if (!in_word) {
                        in_word = 1;
                        countWords++;
                    }
                }
                fclose(file);
            } else {
                perror("wc");
            }
        }
    }

    // If no flags are specified, count everything
    if (!countLinesFlag && !countWordsFlag && !countCharsFlag) {
        countLinesFlag = countWordsFlag = countCharsFlag = 1;
    }

    // Print counts
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "-c") == 0) {
            continue; // Skip flags
        }
        printf("%s: ", argv[i]);
        if (countLinesFlag) printf("%d ", countLines);
        if (countWordsFlag) printf("%d ", countWords);
        if (countCharsFlag) printf("%d ", countChars);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    const char *dirName = "."; // Default directory

    if (argc < 2) {
        list_directory(dirName, 0, 0, 0); // Default to simple ls
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            list_directory(dirName, 1, 0, 0);
        } else if (strcmp(argv[i], "-a") == 0) {
            list_directory(dirName, 0, 1, 0);
        } else if (strcmp(argv[i], "-R") == 0) {
            list_directory(dirName, 0, 0, 1);
        } else if (strcmp(argv[i], "rm") == 0) {
            delete_files(argc - i, argv + i); // Pass remaining arguments to rm
            break; // Exit after handling rm
        } else if (strcmp(argv[i], "cat") == 0) {
            concatenate_files(argc - i, argv + i); // Pass remaining arguments to cat
            break; // Exit after handling cat
        } else if (strcmp(argv[i], "kill") == 0) {
            send_signals(argc - i, argv + i); // Pass remaining arguments to kill
            break; // Exit after handling kill
        } else if (strcmp(argv[i], "wc") == 0) {
            count_words(argc - i, argv + i); // Pass remaining arguments to wc
            break; // Exit after handling wc
        } else {
            dirName = argv[i];
        }
    }

    return 0;
}

