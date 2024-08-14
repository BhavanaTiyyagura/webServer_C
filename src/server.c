#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#define PORT 8080
#define BACKLOG 10

char* response_generator(char* path);

char* handle_http_request(int fd){

	const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char request_type[5];
    char path[4096];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
    }
    
   request[bytes_recvd] = '\0';
    
   sscanf(request,"%s %s HTTP/1.1",request_type,path);
   //printf("%s %s",request_type,path);
   
   if(strcmp(request_type,"GET")!=0) {
		perror("unable to process request");
		exit(EXIT_FAILURE);
	}
   	
   	return response_generator(path);
   	
   }

int main()
{	
	
	struct sockaddr_in server_addr;
	uint16_t size = sizeof(server_addr);
	
	struct sockaddr_in client_addr;  // Separate struct for client address
    socklen_t client_addr_size = sizeof(client_addr);
	
	uint16_t new_socket,len;
	
	memset((char*)&server_addr,0,size);
	
	char* post_message = NULL;
	
	srand(time(0));
	
	//network is big endian and the host can be either. htons() converts host to network 
	
	server_addr.sin_port = htons(PORT); 
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; //OS sets the network interface
	
	
	/*set up the socket that constantly listens for connections */
	
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	
	if(server_fd < 0)
	{
		perror("Error creating socket\n");
		exit(EXIT_FAILURE);
	}
	
	/* bind the socket to the PORT */
	
	if(bind(server_fd,(struct sockaddr *)&server_addr,size) < 0)
	{
	
		perror("Cannot be binded");
		exit(EXIT_FAILURE);
	}
	
	if(listen(server_fd,BACKLOG)<0)
	{
	
		perror("No connections to be made in listen\n");
		exit(EXIT_FAILURE);
	}
	
	while(1) {
	
		printf("-----SERVER WAITING FOR CONNECTION-----\n");
		fflush(stdout);
		
		new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
		if(new_socket <0 ) {
			
			perror("In accept state");
			exit(EXIT_FAILURE);
			
			}
		
		printf("-----CONNECTION ACCEPTED-----\n");
		fflush(stdout);
		
		post_message = handle_http_request(new_socket);
	
		len = strlen(post_message) + 1;
	
		post_message = (char*)realloc(post_message,sizeof(char)*(len));
	
		write(new_socket,post_message,strlen(post_message));
	
		free(post_message);
		
		close(new_socket);
	
	}
	
	return 0;
}

