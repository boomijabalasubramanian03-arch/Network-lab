#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

struct Packet {
    int sequence;
    int total_frames;
    int test_case;
    char generator[20];
    char data[BUFFER_SIZE];
    char crc[20];
};

void convertToBinary(char text[], char binary[]) {
    int length = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        for (int j = 7; j >= 0; j--) {
            binary[length++] = ((text[i] >> j) & 1) + '0';
        }
    }
    binary[length] = '\0';
}

int verifyCRC(struct Packet p) {
    char binary[BUFFER_SIZE * 8];
    char temp[BUFFER_SIZE * 8 + 100];
    convertToBinary(p.data, binary);
    int dataLength = strlen(binary);
    int genLength = strlen(p.generator);
    strcpy(temp, binary);
    strcat(temp, p.crc);

    for (int i = 0; i < dataLength; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < genLength; j++) {
                if (temp[i + j] == p.generator[j]) temp[i + j] = '0';
                else temp[i + j] = '1';
            }
        }
    }
    for (int i = dataLength; i < dataLength + genLength - 1; i++) {
        if (temp[i] == '1') return 0;
    }
    return 1;
}

int main() {
    int sfd, cfd;
    struct sockaddr_in saddr, caddr;
    socklen_t clen = sizeof(caddr);
    struct Packet packet;
    char response[30];
    int expected_seq = 0;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, 5);

    printf("--- TCP Stop-and-Wait Receiver Server Active ---\n");
    cfd = accept(sfd, (struct sockaddr *)&caddr, &clen);

    while (1) {
        int bytes = recv(cfd, &packet, sizeof(packet), 0);
        if (bytes <= 0) break;

        printf("Received Packet: Sequence=%d, Data='%s'\n", packet.sequence, packet.data);

        // Case 2 Handling: Check for CRC Data Corruption Errors
        if (!verifyCRC(packet)) {
            printf("[Receiver] -> CRC Error verified on Frame %d!\n", packet.sequence);
            sprintf(response, "NACK %d", packet.sequence);
            send(cfd, response, strlen(response), 0);
            printf("\n");
            continue;
        }

        if (packet.sequence == expected_seq) {
            printf("[Receiver] -> Frame %d matches sequence order. Saving payload...\n", packet.sequence);
            sprintf(response, "ACK %d", expected_seq);
            send(cfd, response, strlen(response), 0);
            expected_seq++;
        } else {
            // Case 1 Handling: Drop duplicate out-of-order frames
            printf("[Receiver] -> Frame out-of-order. Dropping packet.\n");
            sprintf(response, "ACK %d", expected_seq - 1);
            send(cfd, response, strlen(response), 0);
        }
        printf("\n");
    }
    close(cfd); close(sfd);
    return 0;
}
