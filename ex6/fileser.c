#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8083
#define BUFFER_SIZE 1024
#define NOT_FOUND_MSG "ERR_FILE_NOT_FOUND"

int main() {
    int server_fd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("File server listening on port %d...\n", PORT);


    while (1) {
        if ((new_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");


        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(new_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            close(new_sock);
            continue;
        }
        buffer[bytes_received] = '\0';
        printf("Client requested file: %s\n", buffer);

        FILE *fp = fopen(buffer, "rb");
        if (fp == NULL) {

            send(new_sock, NOT_FOUND_MSG, strlen(NOT_FOUND_MSG), 0);
            printf("File not found: %s\n", buffer);
            close(new_sock);
            continue;
        }


        size_t bytes_read;
        long total_sent = 0;
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
            if (send(new_sock, buffer, bytes_read, 0) < 0) {
                perror("Send failed");
                break;
            }
            total_sent += bytes_read;
        }

        printf("Sent %ld bytes to client.\n", total_sent);

        fclose(fp);
        close(new_sock);
    }

    close(server_fd);
    return 0;
}
