#include "commando.h"

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
  int echo;
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  char *input = NULL;

  //TODO: make and initialize empty cmdcol

  while(1 {
    printf("@>");
    fgets(input, MAX_LINE, stdin);

    if(input == NULL) {
      break;
    }

    if(echo) {
      printf("%s\n", input);
    }
  
    char *tokens[ARG_MAX];
    int *numTokens = 0;
    parse_into_tokens(input, tokens, numTokens);

    if(numTokens != 0) {
      if(strcmp(tokens[0], "help")) {
        help();
      } else if (strcmp(tokens[0], "exit")) {
        break();
      } else if (strcmp(tokens[0], "list")) {
        
      } else if (strcmp(tokens[0], "pause")) {
        pause_for(tokens[1], tokens[2]);
      } else if (strcmp(tokens[0], "output-for")) {
        
      } else if (strcmp(tokens[0], "output-all")) {
        
      } else if (strcmp(tokens[0], "wait-for")) {
        
      } else if (strcmp(tokens[0], "wait-all")) {
        
      } else {

      }
    }
  }
  return 0;
}

void help() {
  printf("COMMANDO COMMANDS
help               : show this message
exit               : exit the program
list               : list all jobs that have been started giving information on each
pause nanos secs   : pause for the given number of nanseconds and seconds
output-for int     : print the output for given job number
output-all         : print output for all jobs
wait-for int       : wait until the given job number finishes
wait-all           : wait for all jobs to finish
command arg1 ...   : non-built-in is run as a job");
}