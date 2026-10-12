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



int* initialize_buffer(void){
    int *buffer = malloc(bufferSize * sizeof(int));

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

int put_element(int conn_fd, int* buffer){

    pthread_mutex_lock(&mutex);
	while(count == bufferSize){
		//printf("buffer full! \n");
		pthread_cond_wait(&not_full,&mutex);
		}
    
	buffer[in_ptr] = conn_fd;
	in_ptr = (in_ptr + 1) % bufferSize;
    count++;
    printf("Producer: inserted fd: %d count is = %d\n",conn_fd,count);
	//printf("Id is in the buffer!\n");
	pthread_cond_signal(&not_empty);
	//printf("Signal to consume sent!\n");
	pthread_mutex_unlock(&mutex);

    return 0;

}
int get_element(int* buffer){
    pthread_mutex_lock(&mutex);
	while(count == 0){ 
        //printf("Buffer is empty\n");
		pthread_cond_wait(&not_empty,&mutex);
    }
    
    int fd = buffer[out_ptr];
    out_ptr = (out_ptr + 1) % bufferSize;
    count--;
    
    printf("Consumer: removed fd: %d count is = %d\n",fd,count);
	pthread_cond_signal(&not_full);
	pthread_mutex_unlock(&mutex);
    
    return fd;

}


