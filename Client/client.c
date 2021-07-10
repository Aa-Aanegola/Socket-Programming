#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define PORT 8000
#define COMM_SIZE 1024
#define PAGE_SIZE 1024

void clean_string(char *str)
{
	int i, pos = 0;

	for(i = 0; str[i]; ++i)
  	{
    	if(isspace(str[i]) && (i == 0 || isspace(str[i-1])))
    		;
    	else if(isspace(str[i]))
    		str[pos++] = ' ';
    	else
    		str[pos++] = str[i];
  	}

  	str[pos] = '\0';
  	pos--;
  	while(pos>=0 && (str[pos] == ' ' || str[pos] == '\t'))
  		str[pos--] = '\0';
  	str[pos+1] = '\0';
}


int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0;
    int num;
    struct sockaddr_in serv_addr;
    char buffer[PAGE_SIZE] = {0};
    
    char command[COMM_SIZE];


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

	while(1)
	{
		printf("Client > ");
		fgets(command, COMM_SIZE, stdin);
		clean_string(command);

		char *token = strtok(command, " ");
		if(token[strlen(token)-1] == '\n')
			token[strlen(token)-1] = '\0';

		
		if(strcmp(token, "exit") == 0)
		{
			send(sock, "EXIT_IS_A_GO", 12, 0);
			break;
		}
		else if(strcmp(token, "get"))
		{
			printf("Invalid command.\n");
			continue;
		}

		token = strtok(NULL, " ");
		if(token == NULL)
		{
			printf("No file name passed.\n");
			continue;
		}

		while(token != NULL)
		{
			printf("%s\n", token);
			
			if(strcmp(token, "") == 0)
				break;
			// Check if file already exists
			struct stat st;

			char file_name[COMM_SIZE];
			strcpy(file_name, token);

			FILE *check = fopen(file_name, "r");
			if(check != NULL)
			{
				printf("File already exists.\n");
				fclose(check);
				break;
			}

			// Send name of file
			if(send(sock, file_name, strlen(file_name), 0) == 0)
			{
				printf("Connection disrupted.\n");
				return 0;
			}

			bzero(buffer, PAGE_SIZE);

			// Receive size of file
			if(recv(sock, buffer, PAGE_SIZE, 0) == 0)
			{
				printf("Connection disrupted.\n");
				return 0;	
			}
			
			long long size = atoi(buffer);
			if(size == -1)
			{
				printf("File not present in server directory.\n");
				break;
			}

			
			int fd = open(file_name, O_CREAT | O_WRONLY, 0600);
			
			
			if(send(sock, "START_TRANSMISSION", 18, 0) == 0)
			{
				printf("Connection disrupted.\n");
				return 0;
			}

			bzero(buffer, PAGE_SIZE);
			int in_size = 0;

			long long cur_size = in_size;

			while(cur_size < size)
			{
				if((in_size = recv(sock, buffer, PAGE_SIZE, 0)) == 0)
				{	
					printf("Connection disrupted.\n");
					return 0;
				}

				write(fd, buffer, in_size);
				float print_size = ((cur_size*100.0)/(size*1.0));
				
				if(print_size > 100.00)
					print_size = 100.00;
				printf("\rNow at %.2f%% completion", print_size);
				
				cur_size += in_size;
				bzero(buffer, PAGE_SIZE);

				send(sock, "PAGE_RECEIVED", 13, 0);
			}
			printf("\rNow at 100.00%% completion\n");
			close(fd);
			token = strtok(NULL, " ");
		}
	}    

    close(sock);
    return 0;
}
