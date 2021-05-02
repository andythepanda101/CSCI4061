<<<<<<< HEAD
#include "blather.h"
#include "util.c"
#include <unistd.h> // signal handling

server_t newserver;
void SIG_handle(int signum){
  server_shutdown(&newserver);
}

int main(int argc, char* argv[]){
  // server_t: data pertaining to server operations
  /*
  typedef struct {
    char server_name[MAXPATH];    // name of server which dictates file names for joining and logging
    int join_fd;                  // file descriptor of join file/FIFO
    int join_ready;               // flag indicating if a join is available
    int n_clients;                // number of clients communicating with server
    client_t client[MAXCLIENTS];  // array of clients populated up to n_clients

    int time_sec;                 // ADVANCED: time in seconds since server started
    int log_fd;                   // ADVANCED: file descriptor for log
    sem_t *log_sem;               // ADVANCED: posix semaphore to control who_t section of log file
  } server_t;
  */

  // signal handler stuff to shut down server if signal are received
  struct sigaction my_sa = {}; // new signal handler
  my_sa.sa_flags = SA_RESTART;
  my_sa.sa_handler = SIG_handle;

  server_start(&newserver, argv[1], DEFAULT_PERMS); // start server with name passed to main, and default perms

  sigaction(SIGINT, &my_sa, NULL); // signal handler for SIGINT
  sigaction(SIGTERM, &my_sa, NULL); // signal handler for SIGTERM

  // main loop to handler server stuff
  while(1){
    // check the sources to see if anything new has occured
    server_check_sources(&newserver);

    // if there is a new client available to join
    if(server_join_ready(&newserver)){
      // add new client to server (means client joined)
      server_handle_join(&newserver);
    }

    // for each client on the server (each client in newserver's client array)
    // check if they are ready to output some information
    // and call the handle function to handle that info
    for(int ind = 0; ind < newserver.n_clients; ind++){
      if(server_client_ready(&newserver, ind)){
        server_handle_client(&newserver, ind);
        server_broadcast(&newserver, );
      }


    }



  }
}
=======
#include "blather.h"
>>>>>>> 4c8b3c4781f396fab512c46ab9299af78b6ec60b