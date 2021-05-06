#include "blather.h"
#include <errno.h>
char client_name [MAXNAME];
char server_name [MAXPATH];
int to_client_fd;
int to_server_fd;
pthread_t client_thread_id;
pthread_t server_thread_id;
simpio_t simpio_actual = {};
simpio_t *simpio = &simpio_actual;

// thread to read input from terminal
// send messages to server
void *client_thread(void *param) {
    while(!simpio->end_of_input) {
        simpio_reset(simpio);
        iprintf(simpio,"");
        while(!simpio->line_ready && !simpio->end_of_input) {
          simpio_get_char(simpio);
        }
        if(simpio->line_ready) {
            mesg_t message = {};
            mesg_t* send = &message;
            send->kind = BL_MESG;
            strcpy(send->name, client_name);
            strcpy(send->body, simpio->buf);
            write(to_server_fd, send, sizeof(mesg_t));
        }
    }
    iprintf(simpio, "End of Input, Departing\n\n");
    pthread_cancel(server_thread_id); // shutdown thread
    mesg_t dept_message = {};
    mesg_t* departed_mesg = &dept_message;
    departed_mesg->kind = BL_DEPARTED;
    strcpy(departed_mesg->name, client_name);
    write(to_server_fd, departed_mesg, sizeof(mesg_t));
    return NULL;
}

// thread for reading messages from server
// if any of these special messages sent from server, print the apporpriate message
void *server_thread(void *param) {
    mesg_t rmessage = {};
    mesg_t *read_message = &rmessage;
    read_message->kind = 0;
    int ret_bytes;
    do {
        ret_bytes = read(to_client_fd, read_message, sizeof(mesg_t));
        if(ret_bytes > 0) {
            if(read_message->kind == BL_MESG) {
                iprintf(simpio, "[%s] : %s\n", read_message->name, read_message->body);
            } else if(read_message->kind == BL_JOINED) {
                iprintf(simpio, "-- %s JOINED --\n", read_message->name);
            } else if(read_message->kind == BL_DEPARTED) {
                iprintf(simpio, "-- %s DEPARTED --\n", read_message->name);
            } else if(read_message->kind == BL_SHUTDOWN) {
                iprintf(simpio, "!!! server is shutting down !!!\n");
            }
        }
    } while(read_message->kind != BL_SHUTDOWN); // loop until shutdown message received
    pthread_cancel(client_thread_id); // shutdown thread
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Improper usage: 2 arguments required\n");
        printf("Proper usage:\n");
        printf("bl_client [server] [user name]\n");
        exit(0);
    }

    // get client and server input
    strcpy(client_name, argv[2]);
    strcpy(server_name, argv[1]);

    // generate to_client and to_server fifo name for the client
    int pid = getpid();
    char to_client_fname[MAXPATH];
    sprintf(to_client_fname, "%d.client.fifo", pid);
    char to_server_fname[MAXPATH];
    sprintf(to_server_fname, "%d.server.fifo", pid);

    // remove the existing fifos with the same name
    remove(to_client_fname);
    remove(to_server_fname);

    // creating new fifos for to_client and to_server
    mkfifo(to_client_fname, DEFAULT_PERMS);
    mkfifo(to_server_fname, DEFAULT_PERMS);

    // open fifos for client to server, and server to client communication
    to_client_fd = open(to_client_fname, O_RDONLY | O_NONBLOCK);
    to_server_fd = open(to_server_fname, O_RDWR | O_NONBLOCK);

    // open the server join fifo
    char server_name_2[MAXPATH];
    strcpy(server_name_2, argv[1]);
    strcat(server_name_2, ".fifo");
    int join_fd = open(server_name_2, O_WRONLY | O_NONBLOCK);

    // create a join request with the to client and to server fifos
    join_t join_msg = {};
    join_t* join = &join_msg;
    strncpy(join->name, argv[2], MAXNAME);
    strncpy(join->to_client_fname, to_client_fname, MAXPATH);
    strncpy(join->to_server_fname, to_server_fname, MAXPATH);

    // write the join request to the server join fifo
    write(join_fd, &join_msg, sizeof(join_t));

    // set up prompt for client
    char newprompt[MAXNAME];
    strcpy(newprompt, client_name);
    strcat(newprompt, PROMPT);
    simpio_set_prompt(simpio, newprompt);
    simpio_reset(simpio);
    simpio_noncanonical_terminal_mode();

    // create thread for
    pthread_create(&client_thread_id, NULL, client_thread, NULL);
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    // waiting but for threads
    pthread_join(client_thread_id, NULL);
    pthread_join(server_thread_id, NULL);

    simpio_reset_terminal_mode();

    return 0;
  }
