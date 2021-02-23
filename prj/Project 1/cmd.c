#include "commando.h"

// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.
cmd_t *cmd_new(char *argv[]) {
    cmd_t *newCmd = malloc(sizeof(cmd_t)); // allocates proper amount of memory for new command
    strcpy(newCmd->name, argv[0]); // copies the command name from the first argv
    int i = 0; 
    while(argv[i] != NULL) { // hard copies all of the strings in argv
        newCmd->argv[i] = strdup(argv[i]);
        i += 1;
    }
    newCmd->argv[i] = NULL; // terminates argv with NULL
    newCmd->finished = 0; // initializes finished to 0
    snprintf(newCmd->str_status,STATUS_LEN,"%s","INIT"); // initializes str_status to INIT
    newCmd->pid = -1; // initlializes pid to default value
    newCmd->out_pipe[PREAD] = -1; // initlializes out_pipe_write to default value
    newCmd->out_pipe[PWRITE] = -1; // initlializes out_pipe_read to default value
    newCmd->status = -1; // initlializes status to default value
    newCmd->output = NULL; // initlializes output to default value
    newCmd->output_size = -1; // initlializes output_size to default value
    return newCmd; // return new command
}

// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.
void cmd_free(cmd_t *cmd) {
    int i = 0;
    while(cmd->argv[i] != NULL) { // deallocates memory for all of the argv strings
        free(cmd->argv[i]);
        cmd->argv[i] = NULL;
        i += 1;
    }
    if(cmd->output != NULL) { // deallocates memory for output, if it was allocated
        free(cmd->output);
        cmd->output = NULL;
    }
    free(cmd); // deallocates the command itself
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
    int pipe_result = pipe(cmd->out_pipe); // creates parent child pipe
    if(pipe_result != 0) { // error checking for pipe
        perror("Failed to create pipe");
        exit(1);
    }

    pid_t child_pid = fork(); // forks child
    if(child_pid < 0) { // error checking for child
        perror("Failed to fork");
        exit(1);
    }

    if(child_pid == 0) { // child closes read pipe, changes std out to write pipe, and executes command
        close(cmd->out_pipe[PREAD]);
        dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);
        execvp(cmd->name, cmd->argv);
        exit(0);
    }
    cmd->pid = child_pid; // saves child pid
    close(cmd->out_pipe[PWRITE]); // closes parent write pipe
    snprintf(cmd->str_status,STATUS_LEN,"%s","RUN"); // changes str_status of command to RUN
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
    if(cmd->finished != 1) { // checks if the command hasen't already finished
        int status;
        waitpid(cmd->pid, &status, nohang); // checks status, waits for process if DOHANG is passed

        if(WIFEXITED(status)) { // if process is finished
            int retcode = WEXITSTATUS(status); // extract status
            cmd->status = retcode; // save exit code
            snprintf(cmd->str_status,STATUS_LEN,"EXIT(%d)",cmd->status); // set str_status to the exit code
            cmd->finished = 1; // flag command as finished
            cmd_fetch_output(cmd); // now that process has ended, grab and save output
            printf("@!!! %s[#%d]: %s\n", cmd->name,(int) cmd->pid, cmd->str_status); // show alert on screen
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
    int max_size = 1, cur_pos = 0, bytesRead = 0; 
    char *buf = malloc(max_size*sizeof(char)); // allocate buffer size = 1

    while(1) { 
        bytesRead = read(fd, &buf[cur_pos], sizeof(char)*(max_size - cur_pos)); // read in enough bytes to fill buffer
        cur_pos += bytesRead; // add the amount of bytes read from read() to our current position
        if(bytesRead == 0) { // if no bytes are read, the fd is empty and we should exit
            break;
        }
        if(max_size == cur_pos) { // if buffer is full
            max_size *= 2; // increase buffer size
            char *newbuf = realloc(buf, max_size*sizeof(char)); // grab new space equal to new buffer size
            if(newbuf == NULL){ // realloc error checking
                printf("ERROR: reallocation failed\n");
                free(buf);
                exit(1);
            }
            buf = newbuf; // overwrite old buffer pointer with new buffer
        }
    }
    buf[cur_pos] = '\0'; // null terminate buffer
    *nread = cur_pos; // populate the number of bytes read in
    return buf; // return a pointer to the buffer of bytes
}

// If cmd->finished is zero, prints an error message 
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.
void cmd_fetch_output(cmd_t *cmd) {
    if(cmd->finished) { // if command is finished
        cmd->output = read_all(cmd->out_pipe[PREAD], &cmd->output_size); // get all of the output from read pipe
        close(cmd->out_pipe[PREAD]); // close output pipe
        return;
    }
    printf("%s[#%d] not finished yet\n", cmd->name, cmd->pid); // if process isn't finished, say so
}

// Prints the output of the cmd contained in the output field if it is
// non-null. Prints an error message if output is NULL. 
// The message includes the command name and PID.
void cmd_print_output(cmd_t *cmd) {
    if(cmd->output == NULL) { // if output is still default value
        printf("%s[#%d] : output not ready\n", cmd->name, cmd->pid); // say the output isn't ready yet
        return; 
    }
    printf("%s\n",(char *) cmd->output); // else print the output
}
