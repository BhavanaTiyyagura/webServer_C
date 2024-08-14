
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SERVER_FILES "."
#define SERVER_ROOT "."

void get_d20()
{

	char filepath[4096];
    FILE* fptr; 
    
	uint32_t val_generated;
	val_generated = rand() % 20 + 1; // generate between 1 and 20
	
	//printf("Generated value: %d\n", val_generated);  
	//fflush(stdout);
	
	char* output_message = NULL;
	uint32_t len_ouput_message = 0;
	
	
	snprintf(filepath, sizeof(filepath), "%s/d20.html", SERVER_ROOT);
	
	//printf("The new path is: %s\n", filepath);
   // fflush(stdout);
	
	char str1[] = "<!DOCTYPE html>\n\n<html>\n  <head>\n    <title>D20</title>\n  </head>\n\n  <body>\n   <h1> The dice have spoken! </h1>  \n   <h2>Your dice roll is: ";
	char str2[] = " </h2>\n\n  </body>\n</html>";
	
	len_ouput_message = strlen(str1) + strlen(str2) + 3;
	
	output_message = (char*)realloc(output_message,sizeof(char)*len_ouput_message);
	snprintf(output_message,len_ouput_message,"%s%d%s",str1,val_generated,str2);
	
	//printf("The new string is: %s\n", output_message);  // debugging line
	
	//printf("The new string is is: %s\n", input_message);
	
	fptr = fopen(filepath,"w");
		
	fprintf(fptr,"%s",output_message);
	fclose(fptr);
	free(output_message);

	
}
	
char* response_generator(char* path)
{
	FILE* fp;
	uint32_t file_size;
	
	char* final_path = (char*)malloc(sizeof(char)*(strlen(path)+2));
	final_path[0] = '.';
	strcat(final_path,path);
	
	char size[5];
	char* buffer_data = NULL; 
	
	uint32_t len;
	uint32_t size_of_message;
	
	char* message = (char*)malloc(sizeof(char)*100);
	
	if(strcmp(final_path,"./d20.html") == 0){
		get_d20();
    }
	
	fp =  fopen(final_path,"r");
	
	if (path == NULL) {
	
	size_of_message = strlen("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nContent-Type: text/html\r\n") + 1;
	
	message = (char*)realloc(message,sizeof(char)*size_of_message);
	
    strcpy (message, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nContent-Type: text/html\r\n");
    
    return message;
    
    }
	
	if (!fp) {
	
	size_of_message = strlen("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nContent-Type: text/html\r\n") + 1;
	
	message = (char*)realloc(message,sizeof(char)*(size_of_message));
    
    perror("Failed to open text file\n");
    
    strcpy (message, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nContent-Type: text/html\r\n");
    
    return message; 
    
    }
    
    size_of_message = strlen("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ") + 1;
    message = (char*)realloc(message,sizeof(char)*(size_of_message));
    strcpy(message,"HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ");
	 
	fseek(fp,0L,SEEK_END); //move file pointer to the end
	file_size = ftell(fp); 
	
	fseek(fp,0L,SEEK_SET); //set the pointer to the beginning of the file
	
	buffer_data = (char*)malloc(sizeof(char)*(file_size+1));
	fread(buffer_data,sizeof(char),file_size,fp);
	buffer_data[file_size+1]='\0';
	
	fclose(fp);
	
	sprintf(size,"%ld",file_size);
	
	len = size_of_message + strlen(size) + strlen(buffer_data) + 3;
	
	message = (char*)realloc(message,sizeof(char)*(len));
	
	//form the final message
	strcat(message,size);
	strcat(message,"\n\n");
	strcat(message,buffer_data);
	
	//add the null terminator
	message[len-1] = '\0';
	 
	free(buffer_data);
	
	free(final_path);
	
	return message;
	
	
}
