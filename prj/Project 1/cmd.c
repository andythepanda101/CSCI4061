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

// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.
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

// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.
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

// Forks a process and executes command in cmd in the process.
// Changes the str_status field to "RUN" using snprintf().  Creates a
// pipe for out_pipe to capture standard output.  In the parent
// process, ensures that the pid field is set to the child PID. In the
// child process, directs standard output to the pipe using the dup2()
// command. For both parent and child, ensures that unused file
// descriptors for the pipe are closed (write in the parent, read in
// the child).
void cmd_start(cmd_t *cmd) {
    int out_pipe[2];
    int pipe_result = pipe(out_pipe);
    if(pipe_result != 0) {
        perror("Failed to create pipe");
        exit(1);
    }

    pid_t child_pid = fork();
    if(child_pid < 0){
        perror("Failed to fork");
        exit(1);
    }

    if(child_pid == 0) {
        close(out_pipe[PREAD]);
        dup2(out_pipe[PWRITE], STDOUT_FILENO);
        execvp(cmd->name, cmd->argv);
    }
    cmd->pid = child_pid;
    cmd->out_pipe[PWRITE] = out_pipe[PWRITE];
    cmd->out_pipe[PREAD] = out_pipe[PREAD];
    close(out_pipe[PWRITE]);
    snprintf(cmd->str_status,STATUS_LEN,"%s","RUN");
}

// If the finished flag is 1, does nothing. Otherwise, updates the
// state of cmd.  Uses waitpid() and the pid field of command to wait
// selectively for the given process. Passes nohang (one of DOBLOCK or
// NOBLOCK) to waitpid() to cause either non-blocking or blocking
// waits.  Uses the macro WIFEXITED to check the returned status for
// whether the command has exited. If so, sets the finished field to 1
// and sets the cmd->status field to the exit status of the cmd using
// the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
// output buffer for later printing.
//
// When a command finishes (the first time), prints a status update
// message of the form
//
// @!!! ls[#17331]: EXIT(0)
//
// which includes the command name, PID, and exit status.
void cmd_update_state(cmd_t *cmd, int nohang) {
    if(cmd->finished != 1) {
        int status;
        waitpid(cmd->pid, &status, nohang);

        if(WIFEXITED(status)){
            cmd->status = WEXITSTATUS(status);
            cmd->finished = 1;
            cmd_fetch_output(cmd);
            printf("%s[#%d]: EXIT(%d)", cmd->name,(int) cmd->pid, cmd->status);
        }
        else{
            printf("Child terminated abnormally\n");
            exit(1);
        }
    }
}

// Reads all input from the open file descriptor fd. Assumes
// character/text output and null-terminates the character output with
// a '\0' character allowing for printf() to print it later. Stores
// the results in a dynamically allocated buffer which may need to
// grow as more data is read.  Uses an efficient growth scheme such as
// doubling the size of the buffer when additional space is
// needed. Uses realloc() for resizing.  When no data is left in fd,
// sets the integer pointed to by nread to the number of bytes read
// and return a pointer to the allocated buffer. Ensures the return
// string is null-terminated. Does not call close() on the fd as this
// is done elsewhere.
char *read_all(int fd, int *nread) {
    int max_size = 1; int cur_pos = 0;
    char *buf = malloc(max_size*sizeof(char));

    while(1) {
        int bytesRead = read(fd, &buf[cur_pos++], max_size - cur_pos);
        nread += bytesRead;
        if(bytesRead == 0) {
            break;
        }
        if(max_size == cur_pos) {
            max_size *= 2;
            char *newbuf = realloc(buf, max_size*sizeof(char));
            if(new_buf == NULL){                         // check that re-allocation succeeded
                printf("ERROR: reallocation failed\n");    // if not...
                free(buf);                                 // de-allocate current buffer
                exit(1);                                   // bail out
            }
            buf = new_buf;
        }
    }
    return *buf;
}

// If cmd->finished is zero, prints an error message with the format
// 
// ls[#12341] not finished yet
// 
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.
void cmd_fetch_output(cmd_t *cmd) {
    if(cmd->finished) {
        
        return;
    }
    printf("%s[#%d] not finished yet", cmd->name, cmd->pid);
}

// Prints the output of the cmd contained in the output field if it is
// non-null. Prints the error message
// 
// ls[#17251] : output not ready
//
// if output is NULL. The message includes the command name and PID.
void cmd_print_output(cmd_t *cmd) {

}

int main(void) {
    cmd_t myCmd;
    char *argv[4] = {"ls", "-l", NULL, NULL};
    myCmd = *cmd_new(argv);
    cmd_free(&myCmd);
    cmd_start(&myCmd);
    cmd_update_state(&myCmd, 0);
}