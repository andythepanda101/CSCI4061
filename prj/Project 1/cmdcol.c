#include "commando.h"

// Print all cmd elements in the given col structure.  The format of
// the table is
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 0    #17434       0    EXIT(0) 2239 ls -l -a -F
// 1    #17435       0    EXIT(0) 3936 gcc --help
// 2    #17436      -1        RUN   -1 sleep 2
// 3    #17437       0    EXIT(0)  921 cat Makefile
//
// Columns correspond to fields in the following way:
// JOB:      index in the cmdcol_t struct
// PID:      pid from the cmd_t struct
// STAT:     status from the cmd_t struct
// STR_STAT: str_status field from cmd_t
// OUTB:     output_size from the cmd_t struct
// COMMAND:  The contents of cmd->argv[] with a space
//           between each element of the array.
//
// Widths of the fields and justification are as follows
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 1234 #12345678 1234 1234567890 1234 Remaining
// left  left    right      right rigt left
// int   int       int     string  int string
void cmdcol_print(cmdcol_t *col){
  // headers for each category
  printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n");
  // loop through each cmd_t in cmd array from the col structure and print the information below
  for(int i = 0; i < col->size; i++){
    // JOB
    printf("%-4d ",i);
    // #PID
    printf("#%-8d ",col->cmd[i]->pid);
    // STAT
    printf("%4d ",col->cmd[i]->status);
    // STR_STAT
    printf("%10s ",col->cmd[i]->str_status);
    // OUTB
    printf("%4d", col->cmd[i]->output_size);
    // COMMAND printing each command in the argv array of each cmd_t
    for(int j = 0; col->cmd[i]->argv[j] != NULL; j++){
      printf(" %s",col->cmd[i]->argv[j]);
    }
    printf("\n");
  }
}


// Add the given cmd to the col structure. Update the cmd[] array and
// size field. Report an error if adding would cause size to exceed
// MAX_CMDS, the maximum number commands supported.
void cmdcol_add(cmdcol_t *col, cmd_t *cmd){
  if(col->size + 1 > MAX_CMDS){
    printf("ERROR: adding another cmd would exceed MAX_CMDS");
    return;
  }
  col->cmd[col->size] = cmd;
  col->size += 1;
}

// Update each cmd in col by calling cmd_update_state() which is also
// passed the block argument (either NOBLOCK or DOBLOCK)
// idk if this is right or not,
void cmdcol_update_state(cmdcol_t *col, int nohang){
  for(int i = 0; i < col->size; i++){
    cmd_update_state(col->cmd[i],nohang);
  }
}

void cmdcol_freeall(cmdcol_t *col){
  for(int i = 0; i < col->size; i++){
    cmd_free(col->cmd[i]);
  }
}
