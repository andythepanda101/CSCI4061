#include "blather.h"

void *client_thread(void *vargp) {
    simpio_reset(simpio);
    while(simpio.end_of_input != 1) {
        simpio_get_char(simpio);
        if(simpio.line_ready == 1) {
            mesg_t send;
            send->kind = BL_MESG;
            strcpy(send->name, argv[1]);
            strncpy(mesg->body, simpio->buf, MAXLINE);
            write(to_server_fd, send, sizeof(mesg_t));
            simpio_reset(simpio);
        }
    }
    iprintf(simpio, "End of Input, Departing");
    mesg_t departed_mesg;
    departed_mesg->kind = BL_DEPARTED;
    strcpy(departed_mesg->name, argv[1]);
    write(to_server_fd, departed_mesg, sizeof(mesg_t));
    return NULL;
}

void *server_thread(void *vargp) {
    do {
        mesg_t *read_message;
        read(to_client_fd, read_message, sizeof(mesg_t));
        switch(read_message->kind) {
            case BL_MESG:
                iprintf(simpio, "[%d] : %d", read_message->name, read_message->body);
            case BL_JOINED:
                iprintf(simpio, "-- %d JOINED --", read_message->name);
            case BL_DEPARTED:
                iprintf(simpio, "-- %d DEPARTED --", read_message->name);
            case BL_SHUTDOWN:
                iprintf(simpio, "!!! server is shutting down !!!");
        }
    } while(read_message->kind != BL_SHUTDOWN)
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        std::cout << "Improper usage: 2 arguments required" << endl;
        std::cout << "Proper usage:" << endl;
        std::cout << "bl_client [server] [user name]" << endl;
        exit(0);
    }
    
    int pid = getpid();
    char to_client_fname[MAXPATH];
    sprintf(to_client_fname, "%d.client.fifo", pid);
    char to_server_fname[MAXPATH];
    sprintf(to_server_fname, "%d.server.fifo", pid);

    remove(to_client_fname);
    remove(to_server_fname);

    mkfifo(to_client_fname, 0666);
    mkfifo(to_server_fname, 0666);

    int to_client_fd = open(to_client_fname, O_RDONLY | O_NONBLOCK);
    int to_server_fd = open(to_server_fname, O_WRONLY | O_NONBLOCK);

    char server_name_2[MAXPATH];
    strcpy(server_name_2, argv[1]);
    strcat(server_name_2, ".fifo");
    int join_fd = open(server_name_2, O_RDONLY | O_NONBLOCK);

    join_t join;
    strncpy(join->name, argv[2], MAXPATH);
    strncpy(join->to_client_fname, to_client_fname, MAXPATH);
    strncpy(join->to_server_fname, to_server_fname, MAXPATH);

    write(join_fd, join, sizeof(join_t));

    simpio_noncanonical_terminal_mode()
    simpio_t *simpio;
    simpio_set_prompt(simpio, PROMPT);

    pthread_t client_thread_id;
    pthread_t server_thread_id;

    pthread_create(&client_thread_id, NULL, client_thread, NULL);
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    pthread_join(client_thread_id, NULL);
    pthread_join(server_thread_id, NULL);

    simpio_reset_terminal_mode();
    
    return 0;
}