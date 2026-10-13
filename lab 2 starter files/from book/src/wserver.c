#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include <pthread.h>
#include "fifo_buffer.h"
#include <sys/time.h>


char default_root[] = ".";

extern pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;

extern int bufferSize = 5;
extern int buff_count = 0;


struct request_init
{
	int flag_logging;
	char* log_file; // file name
	struct request_info* client_buffer_addr; // fifo buffer
	int thread_id;
	double server_start;
};



//
// ./wserver [-d <basedir>] [-p <portnum>] 
//  prompt> ./wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg] 
//  prompt> ./wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg] [-l logfile]
// ./wserver -d "../../html" -p 8000 -t 2 -b 2 -s "FIFO"
int main(int argc, char *argv[]) {
    int c;
	//default values
    char *root_dir = default_root;
	char *log_file = NULL;
    int port = 10000;
	
	int numOfThreads = 1;

	int flag = 0; //for logging

	//time info
	struct timeval t;
	gettimeofday(&t, NULL);
	double server_start = t.tv_sec + t.tv_usec / 1000000.0;
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:l:")) != -1)
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
	case 's':
		break;
	case 'l':
		flag = 1;
		log_file = optarg;
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
	
	//create a buffer of structures. This would be passed as arguments to each of the thread
	// just because of the thread number/id for logging 

	struct request_init rq_init[numOfThreads];

	//allocate the fized size buffer
	struct request_info* request_buffer = initialize_buffer();

	int rc,i;


	pthread_t threads[numOfThreads];

	for( i=0;i<numOfThreads;i++){
		rq_init[i].thread_id = i;
		rq_init[i].client_buffer_addr = request_buffer;
		rq_init[i].flag_logging = flag;
		rq_init[i].log_file = log_file;
		rq_init[i].server_start = server_start;
		rc = pthread_create(&threads[i], NULL,  wrapper_request_handle, &rq_init[i]);

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

	gettimeofday(&t, NULL);
	double current_time = t.tv_sec + t.tv_usec / 1000000.0;
	double arrival_time = current_time - server_start;

	put_element(conn_fd, arrival_time, request_buffer);
	

	//request_handle(conn_fd);
	//close_or_die(conn_fd);
    }
    return 0;
}


    


 
