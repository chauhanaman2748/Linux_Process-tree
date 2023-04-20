#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define CMD_BUF_SIZE 512
#define PROCESS_INFO_BUF_SIZE 1024
int count=0;


void terminate_process(int pid) {
    if (kill(pid, SIGTERM) == -1) {
       // printf("Failed to terminate process %d\n", pid);
        exit(1);
    } else {
        printf("Terminated process %d\n", pid);
    }
}

int get_elapsed_time(int pid) {
    char command[100];
    snprintf(command, sizeof(command), "ps -p %d -o etime=", pid);
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        //printf("Failed to run command\n");
        return -1;
    }
    char elapsed_time_str[20];
    fgets(elapsed_time_str, sizeof(elapsed_time_str), pipe);
    pclose(pipe);
    elapsed_time_str[strcspn(elapsed_time_str, "\n")] = '\0'; // remove newline character
    int elapsed_time = atoi(elapsed_time_str);
    return elapsed_time;
}

void print_defunct_processes(char *process_id, char *option1, int option2, int len) {
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
                if(len==2){
                   // printf("in len2\n");
                    terminate_process(ppid);
                }
                else if(len==4){
                    if(strcmp(option1,"-b")==0){
                        count=count+1;
                       // printf("count: %d..option2: %d\n", count, option2);
                        if(count==option2){
                            terminate_process(ppid);
                        }
                    }
                    else if(strcmp(option1,"-t")==0){
                        int ti = get_elapsed_time(ppid);
                        //printf("%d\n",ti);
                        if(ti>=option2){
                            terminate_process(ppid);
                        }
                    }
                }
            }
            pclose(pipe3);
        } else {
            // recursively check child process tree
            print_defunct_processes(child_id, option1, option2, len);
        }
    }
    pclose(pipe);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc ==3 || argc > 4) {
        printf("Usage: ztree [root_process] [OPTION1] [OPTION2]\n");
        return 1;
    }
    char *root_pid = argv[1];
    char *option1 = argc > 2 ? argv[2] : NULL;
    char *str = malloc(strlen(argv[3]) + 1); // allocate memory for the string
    strcpy(str, argv[3]); // copy the string from argv[1] to str
    int option2 = argc > 2 ? atoi(str) : 0;
    int len= argc;
   // printf("%d\n", option2);
    print_defunct_processes(root_pid, option1, option2, len);
    return 0;
}

