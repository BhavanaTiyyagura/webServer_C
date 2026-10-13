#ifndef __REQUEST_H__

typedef struct 
{
	int flag_logging;
	char* log_file; // file name
	struct request_info* client_buffer_addr; // fifo buffer
    int thread_id;
	double server_start;
}request_init;

void request_handle(int fd, int thread_id, FILE* fptr, double arrival_time, double request_start, double waiting_time, double server_start);
void* wrapper_request_handle(void* buffer);


#endif // __REQUEST_H__
