#include "commando.h"

/*
typedef struct {
  char   name[NAME_MAX+1];         // name of command like "ls" or "gcc"
  char  *argv[ARG_MAX+1];          // argv for running child, NULL terminated
  pid_t  pid;                      // PID of child
  int    out_pipe[2];              // pipe for child output
  int    finished;                 // 1 if child process finished, 0 otherwise
  int    status;                   // return value of child, -1 if not finished
  char   str_status[STATUS_LEN+1]; // describes child status such as RUN or EXIT(..)
  void  *output;                   // saved output from child, NULL initially
  int    output_size;              // number of bytes in output
} cmd_t;
*/

cmd_t *cmd_new(char *argv[]) {
    cmd_t *newCmd = malloc(sizeof(cmd_t));
    strcpy(newCmd->name, argv[0]);
    int i = -1;
    while(argv[++i] != NULL) {
        newCmd->argv[i] = (char*)malloc(sizeof(char)*NAME_MAX);
        strncpy(newCmd->argv[i], argv[i],NAME_MAX);
    }
    newCmd->argv[i + 1] = NULL;
    newCmd->finished = 0;
    snprintf(newCmd->str_status,STATUS_LEN,"%s","INIT");
    newCmd->pid = -1;
    newCmd->out_pipe[0] = -1;
    newCmd->out_pipe[1] = -1;
    newCmd->status = -1;
    newCmd->output = NULL;
    newCmd->output_size = -1;
    return newCmd;
}

void cmd_free(cmd_t *cmd) {
    int i = -1;
    while(cmd->argv[++i] != NULL) {
        free(cmd->argv[i]);
        cmd->argv[i] = NULL;
    }
    if(cmd->output != NULL) {
        free(cmd->output);
        cmd->output = NULL;
    }
    free(cmd);
}

void cmd_start(cmd_t *cmd);
void cmd_fetch_output(cmd_t *cmd);
void cmd_print_output(cmd_t *cmd);
void cmd_update_state(cmd_t *cmd, int nohang);
char *read_all(int fd, int *nread);

int main(void) {
    cmd_t myCmd;
    char *argv[4] = {"ls", "-l", NULL, NULL};
    myCmd = *cmd_new(argv);
    cmd_free(&myCmd);
}