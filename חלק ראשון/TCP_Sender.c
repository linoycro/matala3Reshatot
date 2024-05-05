#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define FILE_SIZE 2 * 1024 * 1024

char *util_generate_random_data(unsigned int size)
{
    char *buffer = NULL;

    if (size == 0)
    {
        return NULL;
    }

    buffer = (char *)calloc(size, sizeof(char));

    if (buffer == NULL)
    {
        return NULL;
    }

    srand(time(NULL));

    for (unsigned int i = 0; i < size; i++)
    {
        buffer[i] = (char)((unsigned int)rand() % 256);
    }

    return buffer;
}

int main(int argc, char *argv[])
{
    

    int bytes_size = 0;

    char *receiver_ip = 0;
    unsigned short port;
    char *congestion_control_alg = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-ip") == 0)
        {
            receiver_ip = argv[i + 1];
        }
        else if (strcmp(argv[i], "-algo") == 0)
        {
            congestion_control_alg = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            port = atoi(argv[i + 1]);
        }
    }

    char *random_data = util_generate_random_data(FILE_SIZE);

    if (random_data == NULL)
    {
        fprintf(stderr, "Failed to generate random data\n");
        exit(EXIT_FAILURE);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Failed to create socket.\n");
        free(random_data);
        exit(EXIT_FAILURE);
    }

    if (strcmp(congestion_control_alg, "reno") != 0 && strcmp(congestion_control_alg, "cubic") != 0)
    {
        fprintf(stderr, "Invalid congestion control algorithm specified\n");
        free(random_data);
        exit(EXIT_FAILURE);
    }
    socklen_t size = sizeof(congestion_control_alg);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, congestion_control_alg, size) < 0)
    {
        perror("Failed to set congestion control algorithm.\n");
        free(random_data);
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, receiver_ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error connecting to server.\n");
        free(random_data);
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        bytes_size = 0;
  
            
            int curr_bytes = send(sock, random_data, FILE_SIZE, 0);
            if (curr_bytes < 0)
            {
                perror("Failed to send data.\n");
                free(random_data);
                close(sock);
                exit(EXIT_FAILURE);
            }
            else if (curr_bytes == 0)
            {
                perror("Connection closed.\n");
                exit(EXIT_FAILURE);
            }



        printf("files sent\n");

        printf("Do you want to send this file again? Enter 1 for yes, 0 for no.\n");
 

        char buff[2];
        recv(sock, buff, sizeof(buff), 0);
        int answer = 0;
        scanf("%d", &answer);
        if (answer == 0)
        {
            break;
        }
        else if (answer != 1)
        {
            printf("Invalid answer. Enter 1 for yes, 0 for no.\n");
        }
        
        send(sock, buff, sizeof(buff), 0);
        recv(sock, buff, sizeof(buff), 0);
    }

  

    const char *exit_message = "EXIT";
    bytes_size = send(sock, exit_message, strlen(exit_message) + 1, 0);
    if (bytes_size == -1)
    {
        perror("Failed to send exit message.\n");

        free(random_data);
        close(sock);

        return 0;
    }
}