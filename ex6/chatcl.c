#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pid_t pid;


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
    printf("Connected to chat server at %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("You can start chatting (type 'exit' to quit).\n");


    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                printf("\nServer disconnected.\n");
                close(sockfd);
                exit(0);
            }
            buffer[bytes_received] = '\0';
            if (strncmp(buffer, "exit", 4) == 0) {
                printf("\nServer ended the chat.\n");
                close(sockfd);
                exit(0);
            }
            printf("\nServer: %s\nYou: ", buffer);
            fflush(stdout);
        }
    } else {

        while (1) {
            printf("You: ");
            fflush(stdout);
            memset(buffer, 0, BUFFER_SIZE);
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
            buffer[strcspn(buffer, "\n")] = '\0'; // strip newline

            send(sockfd, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "exit", 4) == 0) {
                printf("You ended the chat.\n");
                break;
            }
        }
        kill(pid, SIGKILL);
        wait(NULL);
        close(sockfd);
    }

    return 0;
}
