#include <stdio.h>
#include <pthread.h>
#include "fifo_buffer.h"
#include <stdlib.h>

extern int bufferSize;
extern pthread_mutex_t mutex;

extern pthread_cond_t not_empty;
extern pthread_cond_t not_full;

int in_ptr;
int out_ptr;
int count;



struct request_info* initialize_buffer(void){
    struct request_info* buffer = malloc(bufferSize * sizeof(struct request_info));

    if(buffer == NULL){
        fprintf(stderr, "Allocation of buffer failed\n");
        exit(1);
    }

    count = 0;
    in_ptr = 0;
    out_ptr = 0;
    //printf("initialized buffer\n");
    return buffer;
}

int put_element(int conn_fd, double t, struct request_info* buffer){

    pthread_mutex_lock(&mutex);
	while(count == bufferSize){
		//printf("buffer full! \n");
		pthread_cond_wait(&not_full,&mutex);
		}
    
	buffer[in_ptr].fd = conn_fd;
    buffer[in_ptr].arrival_time = t;
	in_ptr = (in_ptr + 1) % bufferSize;
    count++;
    //printf("Producer: inserted fd: %d count is = %d\n",conn_fd,count);
	//printf("Id is in the buffer!\n");
	pthread_cond_signal(&not_empty);
	//printf("Signal to consume sent!\n");
	pthread_mutex_unlock(&mutex);

    return 0;

}
struct request_info get_element(struct request_info* buffer){
    struct request_info return_struct;
    pthread_mutex_lock(&mutex);
	while(count == 0){ 
        //printf("Buffer is empty\n");
		pthread_cond_wait(&not_empty,&mutex);
    }
    
    return_struct.fd = buffer[out_ptr].fd;
    return_struct.arrival_time = buffer[out_ptr].arrival_time;
    out_ptr = (out_ptr + 1) % bufferSize;
    count--;
    
    //printf("Consumer: removed fd: %d count is = %d\n",fd,count);
	pthread_cond_signal(&not_full);
	pthread_mutex_unlock(&mutex);
    
    return return_struct;

}


