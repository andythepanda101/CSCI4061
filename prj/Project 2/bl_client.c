#include "blather.h"
#include <errno.h>
char client_name [MAXNAME];
char server_name [MAXPATH];
int to_client_fd;
int to_server_fd;
pthread_t client_thread_id;
pthread_t server_thread_id;
simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;

void *client_thread(void *param) {
    while(!simpio->end_of_input) {
        simpio_reset(simpio);
        iprintf(simpio,"");
        while(!simpio->line_ready && !simpio->end_of_input) {
          simpio_get_char(simpio);
        }
        if(simpio->line_ready) {
            mesg_t message;
            mesg_t* send = &message;
            send->kind = 10;
            strcpy(send->name, server_name);
            strncpy(send->body, simpio->buf, MAXLINE);
            write(to_server_fd, send, sizeof(mesg_t));
        }
    }
    iprintf(simpio, "End of Input, Departing");
    mesg_t dept_message;
    mesg_t* departed_mesg = &dept_message;
    departed_mesg->kind = BL_DEPARTED;
    strcpy(departed_mesg->name, server_name);
    write(to_server_fd, departed_mesg, sizeof(mesg_t));
    pthread_cancel(server_thread_id);
    return NULL;
}

void *server_thread(void *param) {
    mesg_t rmessage;
    mesg_t *read_message = &rmessage;
    int ret_bytes;
    do {
        ret_bytes = read(to_client_fd, read_message, sizeof(mesg_t));
        if(ret_bytes > 0) {
            iprintf(simpio, "%d /%d",ret_bytes, sizeof(mesg_t));
            if(read_message->kind == BL_MESG) {
                iprintf(simpio, "[%d] : %d\n", read_message->name, read_message->body);
            } else if(read_message->kind == BL_JOINED) {
                iprintf(simpio, "-- %s JOINED --\n", read_message->name);
            } else if(read_message->kind == BL_DEPARTED) {
                iprintf(simpio, "-- %s DEPARTED --\n", read_message->name);
            } else if(read_message->kind == BL_SHUTDOWN) {
                iprintf(simpio, "!!! server is shutting down !!!\n");
            }
        }
    } while(read_message->kind != BL_SHUTDOWN);
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Improper usage: 2 arguments required\n");
        printf("Proper usage:\n");
        printf("bl_client [server] [user name]\n");
        exit(0);
    }

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

    to_client_fd = open(to_client_fname, O_RDONLY | O_NONBLOCK);
    to_server_fd = open(to_server_fname, O_WRONLY | O_NONBLOCK);

    char server_name_2[MAXPATH];
    strcpy(server_name_2, argv[1]);
    strcat(server_name_2, ".fifo");
    int join_fd = open(server_name_2, O_WRONLY | O_NONBLOCK);
    printf("%d\n", join_fd);

    join_t join_msg;
    join_t* join = &join_msg;
    strncpy(join->name, argv[2], MAXNAME);
    strncpy(join->to_client_fname, to_client_fname, MAXPATH);
    strncpy(join->to_server_fname, to_server_fname, MAXPATH);

    int r = write(join_fd, &join_msg, sizeof(join_t));
    printf("%d\n", r);
    printf("%d\n", errno);

    simpio_set_prompt(simpio, PROMPT);
    simpio_reset(simpio);
    simpio_noncanonical_terminal_mode();


    pthread_create(&client_thread_id, NULL, client_thread, NULL);
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    pthread_join(client_thread_id, NULL);
    pthread_join(server_thread_id, NULL);

    simpio_reset_terminal_mode();

    return 0;
  }
