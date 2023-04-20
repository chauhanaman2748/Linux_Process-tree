#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_PATH 1024
#define CMD_BUF_SIZE 512
#define PROCESS_INFO_BUF_SIZE 1024

// Function declarations
void list_process_info(char *pid, char *ppid, char *defunct);
void list_child_processes(char *parent_pid, char *option);
void list_sibling_processes(char *ppid, char *process_id2);
void list_grandparent_process(char *ppid);
void list_grandchild_processes(char *parent_pid, char *option);
int list_defunct_status(char *pid, char *option);
void list_defunct(char *parent_pid);


int main(int argc, char **argv) {
    // Check for correct number of arguments
    if (argc < 2) {
        printf("Usage: prctree [root_process] [process_id]\n");
        return 1;
    }
    char *root_pid = argv[1];
    char *process_id = argv[2];
    char *option = argc > 3 ? argv[3] : NULL;
    char *process_id2 = malloc(strlen(process_id) + 1);
    strcpy(process_id2, process_id);
    //printf("%s\n",process_id2);
    char proc_path[MAX_PATH];
    snprintf(proc_path, MAX_PATH, "/proc/%s", process_id);

    // Check if process_id is a descendant of root_process
    char ppid[MAX_PATH];
    while (strcmp(process_id, root_pid) != 0) {
        // Read parent PID from procfs
        snprintf(proc_path, MAX_PATH, "/proc/%s/stat", process_id);
        FILE *stat_file = fopen(proc_path, "r");
        if (stat_file == NULL) {
           //printf("Error: could not read /proc/%s/stat\n", process_id);
            return 1;
        }
        int status = fscanf(stat_file, "%*d %*s %*c %s", ppid);
        fclose(stat_file);
        if (status != 1) {
           //printf("Error: could not parse /proc/%s/stat\n", process_id);
            return 1;
        }
        if (strcmp(ppid, "1") == 0) {
           //printf("Error: %s is not a descendant of %s\n", process_id, root_pid);
            return 1;
        }
        strcpy(process_id, ppid);
    }

    snprintf(proc_path, MAX_PATH, "/proc/%s/stat", process_id2);
    FILE *stat_file = fopen(proc_path, "r");
    if (stat_file == NULL) {
       //printf("Error: could not read /proc/%s/stat\n", process_id2);
        return 1;
    }
    int status = fscanf(stat_file, "%*d %*s %*c %s", ppid);
    fclose(stat_file);
    // List process information based on option
    list_process_info(process_id2, ppid, NULL);

    if (option != NULL) {
        if (strcmp(option, "-c") == 0) {
            list_child_processes(process_id2, option);
        }
        else if(strcmp(option, "-s") == 0){
            list_sibling_processes(ppid, process_id2);
        }
        else if(strcmp(option, "-gp") == 0){
            list_grandparent_process(ppid);
        }
        else if(strcmp(option, "-gc") == 0){
            list_grandchild_processes(process_id2, option);
        }
        else if(strcmp(option, "-z") == 0){
            list_defunct_status(process_id2, option);
        }
        else if(strcmp(option, "-zl") == 0){
            list_defunct(process_id2);
        }
    }
    
    return 0;
}

// Lists the PID, PPID, and defunct status of a process
void list_process_info(char *pid, char *ppid, char *defunct) {
   //printf("PID: %s\n", pid);
   //printf("PPID: %s\n", ppid);
   printf("%s %s\n",pid,ppid);
    if (defunct != NULL) {
       //printf("Defunct: %s\n", defunct);
    }
}

void list_child_processes(char *parent_pid, char *option) {
    DIR *dir;
    struct dirent *entry;
    char proc_path[MAX_PATH];
    char ppid[MAX_PATH];

    // Open the "/proc" directory
    dir = opendir("/proc");
    if (dir == NULL) {
       //printf("Error: could not open /proc\n");
        return;
    }

    // Iterate over all processes in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a directory (i.e., a process)
        if (isdigit(entry->d_name[0])) {
            snprintf(proc_path, MAX_PATH, "/proc/%s/stat", entry->d_name);

            // Read the PPID from the process's stat file
            FILE *stat_file = fopen(proc_path, "r");
            if (stat_file == NULL) {
               //printf("Error: could not read %s\n", proc_path);
                continue;
            }
            int status = fscanf(stat_file, "%*d %*s %*c %s", ppid);
            fclose(stat_file);
            if (status != 1) {
               //printf("Error: could not parse %s\n", proc_path);
                continue;
            }

            // Check if the process is a child of the parent process
            if (strcmp(ppid, parent_pid) == 0) {
                if(strcmp(option,"-c")==0){
                    printf("%s\n", entry->d_name);
                }
                else if(strcmp(option,"-gc")==0){
                    printf("%s %s\n", entry->d_name,ppid);
                }
            }
        }
    }

    closedir(dir);
}

void list_sibling_processes(char *parent_pid, char *process_id2) {
    DIR *dir;
    struct dirent *entry;
    char proc_path[MAX_PATH];
    char ppid[MAX_PATH];

    // Open the "/proc" directory
    dir = opendir("/proc");
    if (dir == NULL) {
       //printf("Error: could not open /proc\n");
        return;
    }

    // Iterate over all processes in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a directory (i.e., a process)
        if (isdigit(entry->d_name[0])) {
            snprintf(proc_path, MAX_PATH, "/proc/%s/stat", entry->d_name);

            // Read the PPID from the process's stat file
            FILE *stat_file = fopen(proc_path, "r");
            if (stat_file == NULL) {
               //printf("Error: could not read %s\n", proc_path);
                continue;
            }
            int status = fscanf(stat_file, "%*d %*s %*c %s", ppid);
            fclose(stat_file);
            if (status != 1) {
               //printf("Error: could not parse %s\n", proc_path);
                continue;
            }
            if (strcmp(entry->d_name, process_id2) == 0) {
                continue;
            }

            // Check if the process is a child of the parent process
            if (strcmp(ppid, parent_pid) == 0) {
                printf("%s\n", entry->d_name);
            }
        }
    }

    closedir(dir);
}

void list_grandparent_process(char *ppid) {
    char grandparent_pid[MAX_PATH];
    char proc_path[MAX_PATH];
    snprintf(proc_path, MAX_PATH, "/proc/%s/stat", ppid);
    FILE *stat_file = fopen(proc_path, "r");
    if (stat_file == NULL) {
       //printf("Error: could not read /proc/%s/stat\n", ppid);
        return;
    }
    int status = fscanf(stat_file, "%*d %*s %*c %s", grandparent_pid);
    fclose(stat_file);
    if (status != 1) {
       //printf("Error: could not parse /proc/%s/stat\n", ppid);
        return;
    }
    printf("%s\n", grandparent_pid);
}

void list_grandchild_processes(char *parent_pid, char *option) {
      DIR *dir;
    struct dirent *entry;
    char proc_path[MAX_PATH];
    char ppid[MAX_PATH];

    // Open the "/proc" directory
    dir = opendir("/proc");
    if (dir == NULL) {
       //printf("Error: could not open /proc\n");
        return;
    }

    // Iterate over all processes in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a directory (i.e., a process)
        if (isdigit(entry->d_name[0])) {
            snprintf(proc_path, MAX_PATH, "/proc/%s/stat", entry->d_name);

            // Read the PPID from the process's stat file
            FILE *stat_file = fopen(proc_path, "r");
            if (stat_file == NULL) {
               //printf("Error: could not read %s\n", proc_path);
                continue;
            }
            int status = fscanf(stat_file, "%*d %*s %*c %s", ppid);
            fclose(stat_file);
            if (status != 1) {
               //printf("Error: could not parse %s\n", proc_path);
                continue;
            }

            // Check if the process is a child of the parent process
            if (strcmp(ppid, parent_pid) == 0) {
                list_child_processes(entry->d_name, option);
            }
        }
    }

    closedir(dir);
}

int list_defunct_status(char *pid, char *option) {
    char proc_path[MAX_PATH];
    snprintf(proc_path, MAX_PATH, "/proc/%s/stat", pid);
    FILE *stat_file = fopen(proc_path, "r");
    if (stat_file == NULL) {
       //printf("Error: could not read /proc/%s/stat\n", pid);
        return 2;
    }
    char status[MAX_PATH];
    fscanf(stat_file, "%*d %*s %s", status);
    fclose(stat_file);
    if(strcmp(option,"-z")==0){
         if (strcmp(status, "Z") == 0) {
            printf("Defunct\n");
            return 1; 
        } else {
            printf("NOT Defunct\n");
        }
    }
    else if(strcmp(option,"-zl")==0){
         if (strcmp(status, "Z") == 0) {
            return 1; 
        }
    }
    return 0;
}


void list_defunct(char *process_id) {
    // printf("%d\n",option2);
    char command[CMD_BUF_SIZE];
    int result = snprintf(command, CMD_BUF_SIZE, "pgrep -P %s", process_id);
    if (result < 0 || result >= CMD_BUF_SIZE) {
       // printf("Failed to create command\n");
        exit(1);
    }
    FILE *pipe = popen(command, "r");
    if (!pipe) {
       // printf("Failed to run command\n");
        exit(1);
    }
    char child_id[CMD_BUF_SIZE];
    while (fgets(child_id, CMD_BUF_SIZE, pipe)) {
        // remove newline character from child_id string
        child_id[strcspn(child_id, "\n")] = '\0';
        // check if child process is defunct
        result = snprintf(command, CMD_BUF_SIZE, "ps -o state= -p %s", child_id);
        if (result < 0 || result >= CMD_BUF_SIZE) {
           // printf("Failed to create command\n");
            exit(1);
        }
        FILE *pipe2 = popen(command, "r");
        if (!pipe2) {
           // printf("Failed to run command\n");
            exit(1);
        }
        char state[CMD_BUF_SIZE];
        fgets(state, CMD_BUF_SIZE, pipe2);
        pclose(pipe2);
        if (strncmp(state, "Z", 1) == 0) {
            // print defunct process info
            char process_info[PROCESS_INFO_BUF_SIZE];
            result = snprintf(command, CMD_BUF_SIZE, "ps -p %s -o pid,ppid,stat,cmd", child_id);
            if (result < 0 || result >= CMD_BUF_SIZE) {
               // printf("Failed to create command\n");
                exit(1);
            }
            FILE *pipe3 = popen(command, "r");
            if (!pipe3) {
               // printf("Failed to run command\n");
                exit(1);
            }
            fgets(process_info, PROCESS_INFO_BUF_SIZE, pipe3);
            if (fgets(process_info, PROCESS_INFO_BUF_SIZE, pipe3) == NULL) {
               // printf("Failed to read process info\n");
            } else {
               // printf("%s\n", process_info);
                char *pid_str, *ppid_str, *stat, *cmd;
                pid_str = strtok(process_info, " ");
                ppid_str = strtok(NULL, " ");
                stat = strtok(NULL, " ");
                cmd = strtok(NULL, "");

                int pid = atoi(pid_str);
                int ppid = atoi(ppid_str);
               // printf("%d...count:%d\n",ppid,count);
               printf("%d\n",pid);
            }
            pclose(pipe3);
        } else {
            // recursively check child process tree
            list_defunct(child_id);
        }
    }
    pclose(pipe);
}


