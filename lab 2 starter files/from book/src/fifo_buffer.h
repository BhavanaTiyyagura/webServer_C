#ifndef __FIFO_BUFFER_H__
#define __FIFO_BUFFER_H__

struct request_info {
    double arrival_time;
    int fd;
};

struct request_info* initialize_buffer(void);
int put_element(int conn_fd, double time_taken, struct request_info* buffer);
struct request_info get_element(struct request_info* buffer);

#endif