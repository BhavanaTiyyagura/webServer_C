#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include "fifo_buffer.h"

char default_root[] = ".";

extern pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;

extern int bufferSize = 5;
extern int buff_count = 0;

//
// ./wserver [-d <basedir>] [-p <portnum>] 
//  prompt> ./wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	//default values
	int numOfThreads = 1;
	
    
    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
		numOfThreads = atoi(optarg);
		break;
	case 'b':
		bufferSize = atoi(optarg);
		break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	if(bufferSize < 0)
		fprintf(stderr, "Unable to intialize buffer. Enter a positive value\n");
	
	if(numOfThreads < 0)
		fprintf(stderr, "Unable to intialize worker threads. Enter a positive value\n");

	//allocate the fized size buffer
	int* buffer = initialize_buffer();
	int rc,i;


	pthread_t threads[numOfThreads];

	for( i=0;i<numOfThreads;i++){
		rc = pthread_create(&threads[i], NULL,  wrapper_request_handle, buffer);
		if (rc)
			fprintf(stderr, "initializing server failed\n");

	}

	printf("About to listen\n");
	fflush(stdout);
    // now, get to work
    int listen_fd = open_listen_fd_or_die(port); //set up the server to listen to requests (gets socket, binds and listens)
    while (1) {
		//printf("In the while loop\n");
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
    //printf("got a connection!\n");
	//put the fd in the buffer
	put_element(conn_fd, buffer);
	

	//request_handle(conn_fd);
	//close_or_die(conn_fd);
    }
    return 0;
}


    


 
