#include "io_helper.h"
#include "request.h"
#include <pthread.h>
#include "fifo_buffer.h"
#include <time.h>


//
// Some of this code stolen from Bryant/O'Halloran
// Hopefully this is not a problem ... :)
//

#define MAXBUF (8192)

extern pthread_mutex_t mutex;

extern pthread_cond_t not_empty;
extern pthread_cond_t not_full;
extern int bufferSize;
extern int buff_count;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;



void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXBUF], body[MAXBUF];
    int rc;
    // Create the body of error message first (have to know its length for header)
    sprintf(body, ""
	    "<!doctype html>\r\n"
	    "<head>\r\n"
	    "  <title>OSTEP WebServer Error</title>\r\n"
	    "</head>\r\n"
	    "<body>\r\n"
	    "  <h2>%s: %s</h2>\r\n" 
	    "  <p>%s: %s</p>\r\n"
	    "</body>\r\n"
	    "</html>\r\n", errnum, shortmsg, longmsg, cause);
    
    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rc = write_or_kill(fd, buf, strlen(buf));

    if(rc == -1)
        return;
    
    
    sprintf(buf, "Content-Type: text/html\r\n");
    rc = write_or_kill(fd, buf, strlen(buf));

    if(rc == -1)
        return;
    
    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    rc = write_or_kill(fd, buf, strlen(buf));

    if(rc == -1)
        return;
    
    // Write out the body last
    rc = write_or_kill(fd, body, strlen(body));

    if(rc == -1)
        return;
}

//
// Reads and discards everything up to an empty text line
//
int request_read_headers(int fd) {
    char buf[MAXBUF];
    
    int rc;
    rc = readline_or_kill(fd, buf, MAXBUF);
    if(rc == -1 || rc == 0){
        return -1;
    }

    while (strcmp(buf, "\r\n")) {
	rc = readline_or_kill(fd, buf, MAXBUF);
    if(rc == -1 || rc == 0)
        return rc;

    }
    return 0;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int request_parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;
    
    if (!strstr(uri, "cgi")) { 
	// static
	strcpy(cgiargs, "");
	sprintf(filename, ".%s", uri);
	if (uri[strlen(uri)-1] == '/') {
	    strcat(filename, "index.html");
	}
	return 1;
    } else { 
	// dynamic
	ptr = index(uri, '?');
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	} else {
	    strcpy(cgiargs, "");
	}
	sprintf(filename, ".%s", uri);
	return 0;
    }
}

//
// Fills in the filetype given the filename
//
void request_get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html")) 
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif")) 
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg")) 
	strcpy(filetype, "image/jpeg");
    else 
	strcpy(filetype, "text/plain");
}

void request_serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXBUF], *argv[] = { NULL };
    
    // The server does only a little bit of the header.  
    // The CGI script has to finish writing out the header.
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n");
    
    int rc = write_or_kill(fd, buf, strlen(buf));
    if(rc == -1)
        return;
    
    if (fork_or_die() == 0) {                        // child
	setenv_or_die("QUERY_STRING", cgiargs, 1);   // args to cgi go here
	dup2_or_die(fd, STDOUT_FILENO);              // make cgi writes go to socket (not screen)
	extern char **environ;                       // defined by libc 
	execve_or_die(filename, argv, environ);
    } else {
	wait_or_die(NULL);
    }
}

void request_serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    int rc;
    char *srcp, filetype[MAXBUF], buf[MAXBUF];
    
    request_get_filetype(filename, filetype);
    srcfd = open_or_die(filename, O_RDONLY, 0);
    
    // Rather than call read() to read the file into memory, 
    // which would require that we allocate a buffer, we memory-map the file
    srcp = mmap_or_die(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close_or_die(srcfd);
    
    // put together response
    sprintf(buf, ""
	    "HTTP/1.0 200 OK\r\n"
	    "Server: OSTEP WebServer\r\n"
	    "Content-Length: %d\r\n"
	    "Content-Type: %s\r\n\r\n", 
	    filesize, filetype);
    
    rc = write_or_kill(fd, buf, strlen(buf));

     if(rc == -1){
        munmap_or_die(srcp, filesize);
        return;
    }
    
    //  Writes out to the client socket the memory-mapped file 
    rc = write_or_kill(fd, srcp, filesize);
    if(rc == -1){
        munmap_or_die(srcp, filesize);
        return;
    }
        
    munmap_or_die(srcp, filesize);
}

void* wrapper_request_handle(void* arg){
    
    request_init *thread_init = (request_init *)arg;
    FILE* fptr = NULL;
    struct timeval t;

    if(thread_init->flag_logging){
        
        fptr = fopen(thread_init->log_file,"a"); 
        //DEFINE BUFFER
    }

    struct request_info req_info;
    //34.0002s [Thread Main] Arrived - request: GET /index.html
    //34.0010s [Thread 2] Started - request: GET /index.html (0.0008s waiting)

    while(1){

        req_info = get_element(thread_init->client_buffer_addr);
        gettimeofday(&t, NULL);
	    double current_time = t.tv_sec + t.tv_usec / 1000000.0;
        double request_startTime = current_time - thread_init->server_start;
        double waiting_time = request_startTime - req_info.arrival_time;
        double server_start = thread_init->server_start;

        //constructing log
        
        //fprintf(fptr, "\nHi everybody!");
        request_handle(req_info.fd,thread_init->thread_id,fptr,req_info.arrival_time,request_startTime, waiting_time, server_start);
    }
    //printf("In wrapper function\n");

}
// handle a request
void request_handle(int fd,int thread_id, FILE* fptr, double arrival_time, double request_start_time, double waiting_time, double server_start ) {
    int is_static;
    struct stat sbuf;
    char buf[MAXBUF], method[MAXBUF], uri[MAXBUF], version[MAXBUF];
    char filename[MAXBUF], cgiargs[MAXBUF];
    int rc = readline_or_kill(fd, buf, MAXBUF);

    if(rc == -1){
        printf("Connection closed unexpectedly\n");
        close_or_die(fd);
        return;
    }

    else if(rc == 0){ //empty request
        close_or_die(fd);
        return;
    }
    struct timeval t;

    sscanf(buf, "%s %s %s", method, uri, version);
    // LOGFILE started the request 
    if(fptr != NULL){
        //printf("%.4fs [Thread %d] Started - request: %s %s (%.4fs waiting)\n",
        //request_start_time,thread_id,method,uri,
        //waiting_time);

        pthread_mutex_lock(&file_mutex);

        fprintf(fptr, "%.4fs [Thread Main] Arrived - request: %s %s\n",
        arrival_time,method,uri);
        fflush(fptr);
        fprintf(fptr, "%.4fs [Thread %d] Started - request: %s %s (%.4fs waiting)\n",
        request_start_time,thread_id,method,uri,
        waiting_time);
        fflush(fptr);

        pthread_mutex_unlock(&file_mutex);
        //printf("write to file request completed for thread %d\n", thread_id);
    }

    //printf("method:%s uri:%s version:%s\n", method, uri, version);
    
    //post or put
    if (strcasecmp(method, "GET")) {
	request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
	return;
    }
    int rp = request_read_headers(fd);
    if(rp == -1){
        close_or_die(fd);
        return;
    }
    
    is_static = request_parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
	request_error(fd, filename, "404", "Not found", "server could not find this file");
	close_or_die(fd);
    return;
    }
    
    if (is_static) {
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
	    request_error(fd, filename, "403", "Forbidden", "server could not read this file");
	    close_or_die(fd);
        return;
	}
	request_serve_static(fd, filename, sbuf.st_size);
    } else {
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
        request_error(fd, filename, "403", "Forbidden", "server could not run this CGI program");
	    close_or_die(fd);
        return;
	}
	request_serve_dynamic(fd, filename, cgiargs);
    }

    gettimeofday(&t, NULL);
	double current_time = t.tv_sec + t.tv_usec / 1000000.0;
    double completion_time = current_time - server_start;
    double total_time = completion_time - arrival_time; 

    // LOGFILE completed the request 
    if(fptr != NULL){
        //printf("%.4fs [Thread %d] Completed - request: %s %s (%.4fs total)\n",
        //completion_time,thread_id,method,uri,
        //total_time);

        pthread_mutex_lock(&file_mutex);
        fprintf(fptr, "%.4fs [Thread %d] Completed - request: %s %s (%.4fs total)\n",
        completion_time,thread_id,method,uri,
        total_time);
        fflush(fptr);
        

        pthread_mutex_unlock(&file_mutex);
        //printf("write to file completion completed for thread %d\n", thread_id);
    }
    close_or_die(fd);
}
