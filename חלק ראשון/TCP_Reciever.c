#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024 * 1024 * 2
#define MAX_TIMES 100

double calculate_bandwidth(double time_diff, int data_size)
{
    double bandwidth = ((data_size / 1024) / 1024) / (time_diff / 1000);
    return bandwidth;
}

int main(int argc, char *argv[])
{
    double sum_time = 0;
    double sum_bandwidth = 0;
    double bandwidth[MAX_TIMES];
    double times[MAX_TIMES];
    int timecount = 0;
    char buffer[BUFFER_SIZE] = {0};
    int bytes_size = 0;
    struct timeval stop, start;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }
    unsigned short port;
    char *congestion_control_alg;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-algo") == 0)
        {
            congestion_control_alg = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            port = atoi(argv[i + 1]);
        }
    }

    struct sockaddr_in sender_addr, rcv_addr;
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(port);
    sender_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) == -1)
    {
        perror("Error binding socket.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 1) == -1)
    {
        perror("Error listening.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    int rvc_len = sizeof(rcv_addr);
    int rcv_sock = accept(sock, (struct sockaddr *)&rcv_addr, &rvc_len);
    if (rcv_sock == -1)
    {
        perror("Error accepting connection.\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        bytes_size = 0;
        gettimeofday(&start, NULL);
        

        while (bytes_size < 2 * 1024 * 1024)
        {
            int curr_bytes = recv(rcv_sock, buffer + bytes_size, BUFFER_SIZE - bytes_size, 0);

            if (curr_bytes == -1)
            {
                perror("Error receiving file.\n");
                close(rcv_sock);
                close(sock);
                exit(EXIT_FAILURE);
            }
            else if (curr_bytes == 0)
            {
                perror("Connection closed.\n");
                exit(EXIT_FAILURE);
            }

            bytes_size += curr_bytes;
        }

        gettimeofday(&stop, NULL);
        printf("got the file!\n");
        double calc_time = (stop.tv_sec - start.tv_sec) * 1000.0 + (stop.tv_usec - start.tv_usec) / 1000.0;
        times[timecount] = calc_time;
        bandwidth[timecount] = calculate_bandwidth(calc_time, bytes_size);
        timecount++;

        send(rcv_sock, "\0", 2, 0);
        char buffer[16] = { 0 };
        recv(rcv_sock, buffer, sizeof(buffer), 0);

        if (strcmp("EXIT", buffer) == 0)
        {
            close(rcv_sock);
            close(sock);
            break;
        }

        send(rcv_sock, "\0", 2, 0);
    }

    for (int i = 0; i < timecount; i++)
    {
        sum_time += times[i];
        sum_bandwidth += bandwidth[i];
        printf("Run #%d Data: Time=%.2fms; Speed=%.2fMB/s\n", i + 1, times[i], bandwidth[i]);
    }

    double avg_time = sum_time / timecount;
    double avg_bandwidth = sum_bandwidth / timecount;

    printf("Average time: %.2fms\n", avg_time);
    printf("Average bandwidth: %.2fMB/s\n", avg_bandwidth);
    close(rcv_sock);
    close(sock);

    return 0;
}