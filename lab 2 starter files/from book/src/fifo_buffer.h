#ifndef __FIFO_BUFFER_H__

int* initialize_buffer(void);
int put_element(int conn_fd, int* buffer);
int get_element(int* buffer);

#endif