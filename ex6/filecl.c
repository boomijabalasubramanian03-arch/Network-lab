#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8083
#define BUFFER_SIZE 1024
#define NOT_FOUND_MSG "ERR_FILE_NOT_FOUND"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char filename[256];
    char save_path[300];


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to file server at %s:%d\n", SERVER_IP, SERVER_PORT);


    printf("Enter filename to download: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0'; // strip newline


    if (send(sockfd, filename, strlen(filename), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Server closed the connection.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bytes_received == (ssize_t)strlen(NOT_FOUND_MSG) &&
        strncmp(buffer, NOT_FOUND_MSG, bytes_received) == 0) {
        printf("Server error: file '%s' was not found.\n", filename);
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    snprintf(save_path, sizeof(save_path), "received_%s", filename);
    FILE *fp = fopen(save_path, "wb");
    if (fp == NULL) {
        perror("Could not create local file");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    long total_received = 0;


    fwrite(buffer, 1, bytes_received, fp);
    total_received += bytes_received;

    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, fp);
        total_received += bytes_received;
    }

    printf("File received successfully: %s (%ld bytes)\n", save_path, total_received);

    fclose(fp);
    close(sockfd);
    return 0;
}
