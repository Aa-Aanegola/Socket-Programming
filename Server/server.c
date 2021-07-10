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

#define PORT 8000
#define PAGE_SIZE 1024

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[PAGE_SIZE] = {0};
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    while(1)
    {
        if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        while(1)
        {
            bzero(buffer, PAGE_SIZE);
            if(recv(new_socket, buffer, sizeof(buffer), 0) == 0)
            {
                printf("Connection disrupted.\n");
                break;
            }
            

            // If we got an exit command, then we exit
            if(strcmp(buffer, "EXIT_IS_A_GO") == 0)
                break;

            // Otherwise we got a file name 
            int fd = open(buffer, O_RDONLY);

            if(fd == -1)
            {
                send(new_socket, "-1", 2, 0);
            }
            else
            {
                bzero(buffer, sizeof(buffer));

                long long int size = lseek(fd, 0, SEEK_END);
                lseek(fd, 0, 0);
                sprintf(buffer, "%lld", size);
                

                if(send(new_socket, buffer, sizeof(buffer), 0) == 0)
                {
                    printf("Connection disrupted.\n");
                    break;
                }

                bzero(buffer, sizeof(buffer));

                if(recv(new_socket, buffer, sizeof(buffer), 0) == 0)
                {
                    printf("Connection disrupted.\n");
                    break;
                }

                if(strcmp(buffer, "START_TRANSMISSION") != 0)
                {
                    printf("Failed to connect.\n");
                    continue;
                }

                bzero(buffer, sizeof(buffer));


                int read_size;
                while((read_size = read(fd, buffer, sizeof(buffer))) != 0)
                {
                    if(send(new_socket, buffer, read_size, 0) == 0)
                    {
                        printf("Connection disrupted.\n");
                        break;
                    }
                    
                    bzero(buffer, sizeof(buffer));

                    if(recv(new_socket, buffer, sizeof(buffer), 0) == 0)
                    {
                        printf("Connection disrupted.\n");
                        break;
                    }
                    if(strcmp(buffer, "PAGE_RECEIVED"))
                    {
                        printf("Connection disrupted.\n");
                        break;
                    }
                    bzero(buffer, sizeof(buffer));
                }
                bzero(buffer, sizeof(buffer));
            }
            bzero(buffer, sizeof(buffer));
            close(fd);
        }

        close(new_socket);
    }
    return 0;
}
