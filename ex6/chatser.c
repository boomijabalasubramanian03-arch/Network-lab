#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    pid_t pid;

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

    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Chat server listening on port %d...\n", PORT);

    if ((new_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client connected. You can start chatting (type 'exit' to quit).\n");


    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        close(new_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_received = recv(new_sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                printf("\nClient disconnected.\n");
                close(new_sock);
                exit(0);
            }
            buffer[bytes_received] = '\0';
            if (strncmp(buffer, "exit", 4) == 0) {
                printf("\nClient ended the chat.\n");
                close(new_sock);
                exit(0);
            }
            printf("\nClient: %s\nYou: ", buffer);
            fflush(stdout);
        }
    } else {

        while (1) {
            printf("You: ");
            fflush(stdout);
            memset(buffer, 0, BUFFER_SIZE);
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
            buffer[strcspn(buffer, "\n")] = '\0'; // strip newline

            send(new_sock, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "exit", 4) == 0) {
                printf("You ended the chat.\n");
                break;
            }
        }
        kill(pid, SIGKILL);
        wait(NULL);
        close(new_sock);
        close(server_fd);
    }

    return 0;
}
