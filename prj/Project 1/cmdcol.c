#include "commando.h"

void cmdcol_print(cmdcol_t *col);
void cmdcol_add(cmdcol_t *col, cmd_t *cmd);
void cmdcol_update_state(cmdcol_t *col, int nohang);
void cmdcol_freeall(cmdcol_t *col);
