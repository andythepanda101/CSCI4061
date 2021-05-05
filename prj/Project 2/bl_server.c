#include "blather.h"
//#include "util.c"
#include <unistd.h> // signal handling

server_t newserver;
void SIG_handle(int signum){
  server_shutdown(&newserver);
  printf("\n");
  exit(0);
}

int main(int argc, char* argv[]){

  // if a server name is not specified in the args, or too many args
  if(argc != 2){
    printf("Expected 1 argument\nUsage: ./bl_server <insert_name_here>\n");
    exit(0);
  }

  // signal handler stuff to shut down server if signal are received
  struct sigaction my_sa = {}; // new signal handler
  //sigemptyset(&my_sa.sa_mask);
  my_sa.sa_flags = SA_RESTART;
  my_sa.sa_handler = SIG_handle;

  server_start(&newserver, argv[1], O_RDONLY | O_NONBLOCK); // start server with name passed to main, and default perms

  sigaction(SIGINT, &my_sa, NULL); // signal handler for SIGINT
  sigaction(SIGTERM, &my_sa, NULL); // signal handler for SIGTERM

  // main loop to handler server stuff
  while(1){
    // check the sources to see if anything new has occured
    server_check_sources(&newserver);
    //printf("test");  // debug
    // if there is a new client available to join
    if(server_join_ready(&newserver)){
      // handle the join request by adding new client to server (means client joined)
      server_handle_join(&newserver);
    }

    // for each client on the server (each client in newserver's client array)
    // check if they are ready to output some information
    // and call the handle function to handle that info
    for(int ind = 0; ind < newserver.n_clients; ind++){
      if(server_client_ready(&newserver, ind)){
        server_handle_client(&newserver, ind);
        /*
        mesg_t *mesg;
        read(server.client[ind].to_server_fd, mesg, sizeof(mesg_t));
        if(mesg->kind == 30 | mesg->kind == 40) {
            server_remove_client(&newserver, i);
        }
        */
      }
    }
  }

  return 0;
}
