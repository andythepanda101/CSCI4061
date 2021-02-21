#include "commando.h"

void print_output(cmdcol_t *col, int job) {
  cmd_fetch_output(col->cmd[job]);
  cmd_print_output(col->cmd[job]);
}

void wait_for(cmdcol_t *col, int job) {
  cmd_update_state(col->cmd[job], DOBLOCK);
}

/*
1.Print the prompt @>
2.Use a call to fgets() to read a whole line of text from the user. 
The #define MAX_LINE limits the length of what will be read. If no input is remains, print End of input and break out of the loop.
3.Echo (print) given input if echoing is enabled.
4.Use a call to parse_into_tokens() from util.c to break the line up by spaces. If there are no tokens, jump to the end of the loop (the use just hit enter).
5.Examine the 0th token for built-ins like help, list, and so forth. Use strncmp() to determine if any match and make appropriate calls. 
This will be a long if/else chain of statements.
6.If no built-ins match, create a new cmd_t instance where the tokens are the argv[] for it and start it running.
7.At the end of each loop, update the state of all child processes via a call to cmdcol_update_state().
*/

int main(int argc, char *argv[]){
  int echo = 0;
  if(argc > 1) {
    echo = !strcmp("--echo", argv[1]) || getenv("COMMANDO_ECHO");
  }
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  
  char *input = malloc(NAME_MAX*sizeof(char));
  cmdcol_t *cmdCol = malloc(sizeof(cmdcol_t));

  while(1) {
    printf("@> ");
    fgets(input, MAX_LINE, stdin);

    if(input == NULL) {
      break;
    }

    if(echo) {
      printf("%s\n", input);
    }
  
    char *tokens[ARG_MAX];
    int numTokens = 0;
    parse_into_tokens(input, tokens, &numTokens);

    if(numTokens != 0) {
      if(!strcmp(tokens[0], "help")) {
        printf("COMMANDO COMMANDS\nhelp               : show this message\nexit               : exit the program\nlist               : list all jobs that have been started giving information on each\npause nanos secs   : pause for the given number of nanseconds and seconds\noutput-for int     : print the output for given job number\noutput-all         : print output for all jobs\nwait-for int       : wait until the given job number finishes\nwait-all           : wait for all jobs to finish\ncommand arg1 ...   : non-built-in is run as a job\n");
      } else if (!strcmp(tokens[0], "exit")) {
        break;
      } else if (!strcmp(tokens[0], "list")) {
        cmdcol_print(cmdCol);
      } else if (!strcmp(tokens[0], "pause")) {
        pause_for((long int) tokens[1],(int) *tokens[2]);
      } else if (!strcmp(tokens[0], "output-for")) {
        print_output(cmdCol, atoi(tokens[1]));
      } else if (!strcmp(tokens[0], "output-all")) {
        int i;
        for(i = 0; i < cmdCol->size; i++) {
          print_output(cmdCol, i);
        }
      } else if (!strcmp(tokens[0], "wait-for")) {
        wait_for(cmdCol, atoi(tokens[1]));
      } else if (!strcmp(tokens[0], "wait-all")) {
        cmdcol_update_state(cmdCol, DOBLOCK);
      } else {
        cmd_t *newCmd;
        newCmd = cmd_new(tokens);
        cmd_start(newCmd);
        cmdcol_add(cmdCol, newCmd);
      }
      cmdcol_update_state(cmdCol, NOBLOCK);
    }
  }
  cmdcol_freeall(cmdCol);
  free(input);
  return 0;
}