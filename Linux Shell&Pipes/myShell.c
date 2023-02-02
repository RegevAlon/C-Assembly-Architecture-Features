#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <fcntl.h>

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0
#define STDOUT 1
#define STDIN 0
#define HISTLEN 5

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_proc= (process*)malloc(sizeof(process));
    new_proc->cmd = cmd;
    new_proc -> pid = pid ;
    new_proc -> status = RUNNING;
    new_proc->next = NULL;
    process *curr = *process_list;
    if(curr == NULL) {
        *process_list = new_proc;
    }
    else {
        while (curr->next)
            curr = curr->next;
        curr->next = new_proc;
    }
}

void updateProcessList(process **process_list){
    process *cur = *process_list;
    int status;
    while ( cur != NULL) {
        int returned =waitpid(cur->pid, &status, WNOHANG|WUNTRACED|WCONTINUED);
        if(returned!=0){
            if ( (returned == -1) | WIFEXITED(status))
                cur->status = TERMINATED;
            else if (WIFSTOPPED(status))
                cur->status = SUSPENDED;
            else if (WIFCONTINUED(status))
                cur->status = RUNNING;
        }
        cur = cur->next;
    }
}

void printProcessList(process** process_list){
    updateProcessList(process_list);
    process *curr = *process_list;
    process *prev =NULL;
    process *temp_next =NULL;
    char *status;
    printf("PID\t\tCOMMAND\t\tSTATUS\n");
    while (curr != NULL){
        if(curr->status == TERMINATED)
            status= "TERMINATED";
        else if(curr->status == RUNNING)
            status= "RUNNING";
        else if(curr->status == SUSPENDED)
            status = "SUSPENDED" ;
        printf("%d\t\t%s\t\t%s\n",curr->pid,curr->cmd->arguments[0],status);
        if(curr->status!=TERMINATED){
            prev=curr;
            curr=curr->next;
        }
        else{
            if(prev != NULL){
                prev->next = curr->next;
                temp_next = curr->next;
                freeCmdLines(curr->cmd);
                free(curr);
                curr = temp_next;
            }
            else{
                process_list[0] = curr->next;
                temp_next = curr->next;
                freeCmdLines(curr->cmd);
                free(curr);
                curr = temp_next;
            }
        }
    }
}
void freeProcessList(process* process_list){
    process *curr = process_list;
    while(curr){
        if(curr->cmd) {
            freeCmdLines(curr->cmd);
        }
        process *temp = curr;
        curr = curr->next;
        free(temp);
    }
}


void updateProcessStatus(process* process_list, int pid, int status){
    process *curr = process_list;
    while (curr){
        if (curr->pid == pid) {
            curr->status = status;
        }
        curr = curr->next;
    }
}

int execute(cmdLine *pCmdLine) {
    int x;
    int y;
    pid_t child = fork();
    if (child==0) {
        if (pCmdLine->inputRedirect != NULL) {
            x = open(pCmdLine->inputRedirect, O_RDONLY | O_CREAT, 0644);
            close(STDIN);
            dup2(x, 0);
            //close(x);
        }
        if (pCmdLine->outputRedirect != NULL) {
            y = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT, 0644);
            close(STDOUT);
            dup2(y, 1);
            //close(y);
        }
        int val = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
        if (val == -1) {
            perror("execution error");
            freeCmdLines(pCmdLine);
            exit(1);
        }
    } else if (child == -1) {
        perror("fork error");
        exit(1);
    }
    if (pCmdLine->blocking != 0)
        waitpid(child, NULL, 0);

    return child;
}

int singlePipe(cmdLine *cl,int debug){
    int pipefd[2];
    cmdLine *cl1 = cl;
    cmdLine *cl2 = cl->next;


    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    if(debug==1) {
        fprintf(stderr,"parent_process>forking..\n");
    }
    pid_t child1 = fork();
    if (child1 == -1) {
        perror("fork");
        return 1;
    }
    if(debug==1 && child1) {
        fprintf(stderr, "parent_process>created process with id: %d\n", child1);
    }
    if (child1 == 0) {
        if(debug==1) {
            fprintf(stderr,"child1>redirecting stdout to the write end of the pipe\n");
        }
        if (cl1->inputRedirect != NULL) {
            pipefd[0] = open(cl1->inputRedirect, O_RDONLY | O_CREAT, 0644);
            close(STDIN);
            dup(pipefd[0]);
            close(pipefd[0]);
        }
        close(STDOUT);
        dup(pipefd[1]);
        close(pipefd[1]);
        if(debug==1) {
            fprintf(stderr,"child1>going to execute cmd %s:\n",cl1->arguments[0]);
        }
        execvp(cl1->arguments[0],cl1->arguments);
    } else {
        close(pipefd[1]);
        if(debug==1) {
            fprintf(stderr,"parent_process>forking..\n");
        }
        pid_t child2 = fork();
        if (child2 == -1) {
            perror("fork");
            return 1;
        }
        if(debug==1 && child2) {
            fprintf(stderr, "parent_process>created process with id: %d\n", child2);
        }
        if (child2 == 0) {
            if(debug==1) {
                fprintf(stderr,"child2>redirecting stdin to the read end of the pipe\n");
            }
            close(STDIN);
            dup(pipefd[0]);
            close(pipefd[0]);
            if (cl2->outputRedirect != NULL) {
                pipefd[1] = open(cl2->outputRedirect, O_WRONLY | O_CREAT, 0644);
                close(STDOUT);
                dup(pipefd[1]);
                close(pipefd[1]);
            }
            if(debug==1)
                fprintf(stderr,"child2>going to execute cmd %s\n",cl2->arguments[0]);
            execvp(cl2->arguments[0],cl2->arguments);
        } else {
            if(debug==1) {
                fprintf(stderr,"parent_process>closing the read end of the pipe\n");
            }
            //close(pipefd[0]);
            if(debug==1) {
                fprintf(stderr,"parent_process>waiting for child processes to terminate\n");
            }
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);
        }
        if(debug==1) {
            fprintf(stderr,"parent_process>exiting..\n");
        }
    }
    return 0;
}

int newest_idx = 0;
int oldest_idx = 0;
char *history[HISTLEN];

void addToHistory(char* cl){
    char* cpy_cl = strdup(cl);
    if(newest_idx==oldest_idx && history[newest_idx]!=NULL) {
        free(history[oldest_idx]);
        oldest_idx = (oldest_idx + 1) % HISTLEN;
    }
    history[newest_idx] = cpy_cl;
    printf("oldest index: %d\n",oldest_idx);
    newest_idx = (newest_idx+1)%HISTLEN;
    printf("newest index: %d\n",newest_idx);

}

void printHistory(){
    int index;
    int i;
    for(i=0;i<HISTLEN;i++){
        index = (oldest_idx+i)%HISTLEN;
        if(history[index]!=NULL)
            printf("Command %d: %s",i+1,history[index]);
    }
}

char* retrieveFromHistory(int n) {
    if (n < 1 || n > HISTLEN)
        perror("Invalid history index");
    int index = (oldest_idx+n-1)%HISTLEN;
    if (history[index]!=NULL)
        return history[index];
    else printf("ERROROROROROR");
    return NULL;

}

int main(int argc, char* argv[]) {
    char path[PATH_MAX];
    int pid;
    process **process_list = malloc(sizeof(process));
    process_list[0] = NULL;
    char input[2048];
    char* a;
    int debug = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
        debug = 1;
    while (1) {
        getcwd(path, PATH_MAX);
        printf("%s\n",path);
        fgets(input, 2048, stdin);
        cmdLine *cl = parseCmdLines(input);
        if (strcmp(cl->arguments[0], "procs") == 0) {
            printProcessList(process_list);
            freeCmdLines(cl);
        }
        else if (strcmp(cl->arguments[0], "suspend") == 0) {
            pid = (pid_t )strtol(cl->arguments[1],&a,10);
            updateProcessStatus(*process_list,pid,SUSPENDED);
            if(kill(pid, SIGTSTP))
                printf("%d Fail\n",pid);
            else printf("Success\n");
            freeCmdLines(cl);
        }
        else if (strcmp(cl->arguments[0], "history") == 0) {
            printHistory();
        }
        else if (strcmp(cl->arguments[0], "!!") == 0) {
            char* new_input = retrieveFromHistory(1);
            cmdLine *cl1 = parseCmdLines(new_input);
            if(cl1->next!=NULL)
                singlePipe(cl1,debug);
            else execute(cl1);
            addToHistory(new_input);

        }
        else if (cl->arguments[0][0]=='!') {
            char *ptr;
            long n = strtol(cl->arguments[0] + 1, &ptr, 10);
            char *new_input = retrieveFromHistory(n);
            if (new_input != NULL) {
                cmdLine *cl1 = parseCmdLines(new_input);
                execute(cl1);
                addToHistory(new_input);
                freeCmdLines(cl1);
            }
        }
        else if (strcmp(cl->arguments[0],"cd") == 0) {
            char *newPath = cl->arguments[1];
            chdir(newPath);
        }
        else if (strcmp(cl->arguments[0], "kill") == 0) {
            pid = (pid_t )strtol(cl->arguments[1],&a,10);
            if(kill(pid, SIGINT))
                printf("%d Fail\n",pid);
            else printf("Success\n");
            updateProcessStatus(*process_list,pid,TERMINATED);
            freeCmdLines(cl);
        }
        else if (strcmp(cl->arguments[0], "wake") == 0) {
            pid = (pid_t )strtol(cl->arguments[1],&a,10);
            if(kill(pid, SIGCONT))
                printf("%d Fail\n",pid);
            else printf("Success\n");
            updateProcessStatus(*process_list,pid,RUNNING);
            freeCmdLines(cl);
        }
        else if(strcmp(cl->arguments[0], "quit") == 0){
            freeProcessList(*process_list);
            int i=0;
            for( i=0;i<HISTLEN;i++){
                free(history[i]);
            }
            exit(0);
        }
        else if(cl->next!=NULL){
            singlePipe(cl,debug);
        }
        else {
            pid = execute(cl);
            if (debug == 1)
                printf("PID: %d, Executing command: %s\n", pid, cl->arguments[0]);
            addProcess(process_list, cl, pid);
            addToHistory(input);

        }
        freeCmdLines(cl);

    }
}
