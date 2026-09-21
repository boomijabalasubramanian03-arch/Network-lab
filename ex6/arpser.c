#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define BUFFER_SIZE 1024
#define TABLE_SIZE 5

typedef struct {
    char ip[20];
    char mac[20];
} ARPEntry;

ARPEntry arp_table[TABLE_SIZE] = {
    {"192.168.1.1", "AA:BB:CC:DD:EE:01"},
    {"192.168.1.2", "AA:BB:CC:DD:EE:02"},
    {"192.168.1.3", "AA:BB:CC:DD:EE:03"},
    {"192.168.1.4", "AA:BB:CC:DD:EE:04"},
    {"192.168.1.5", "AA:BB:CC:DD:EE:05"}
};


char* lookup_mac(const char *ip) {
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (strcmp(arp_table[i].ip, ip) == 0) {
            return arp_table[i].mac;
        }
    }
    return NULL;
}

int main() {
    int server_fd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char reply[BUFFER_SIZE];


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
    printf("ARP server listening on port %d...\n", PORT);


    while (1) {
        if ((new_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");


        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_received = recv(new_sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                printf("Client disconnected.\n");
                break;
            }
            buffer[bytes_received] = '\0';

            if (strncmp(buffer, "exit", 4) == 0) {
                printf("Client ended the session.\n");
                break;
            }

            printf("ARP Request for IP: %s\n", buffer);

            char *mac = lookup_mac(buffer);
            if (mac != NULL) {
                snprintf(reply, BUFFER_SIZE, "ARP Reply: %s is at %s", buffer, mac);
            } else {
                snprintf(reply, BUFFER_SIZE, "ARP Reply: MAC not found for %s", buffer);
            }

            send(new_sock, reply, strlen(reply), 0);
            printf("Sent -> %s\n", reply);
        }

        close(new_sock);
    }

    close(server_fd);
    return 0;
}
