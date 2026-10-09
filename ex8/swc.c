#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

struct Packet {
    int sequence;
    int total_frames;
    int test_case; // 0 = Normal, 3 = Missed/Drop, 4 = CRC Error
    char generator[20];
    char data[BUFFER_SIZE];
    char crc[20];
};

struct Node {
    struct Packet packet;
    struct Node* next;
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

void generateCRC(struct Packet *p) {
    char binary[BUFFER_SIZE * 8];
    char temp[BUFFER_SIZE * 8 + 100];
    convertToBinary(p->data, binary);
    int dataLength = strlen(binary);
    int genLength = strlen(p->generator);

    strcpy(temp, binary);
    for (int i = 0; i < genLength - 1; i++) {
        strcat(temp, "0");
    }
    for (int i = 0; i < dataLength; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < genLength; j++) {
                if (temp[i + j] == p->generator[j]) temp[i + j] = '0';
                else temp[i + j] = '1';
            }
        }
    }
    strncpy(p->crc, &temp[dataLength], genLength - 1);
    p->crc[genLength - 1] = '\0';
}

void appendNode(struct Node** head, struct Packet p) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->packet = p;
    newNode->next = NULL;
    if (*head == NULL) { *head = newNode; return; }
    struct Node* temp = *head;
    while (temp->next != NULL) temp = temp->next;
    temp->next = newNode;
}

int main() {
    int cfd, total;
    struct sockaddr_in saddr;
    char response[30];

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    printf("--- Connected to Stop-and-Wait Server ---\n\n");

    printf("Enter total number of frames you want to send: ");
    scanf("%d", &total);
    getchar(); // Clear newline buffer

    struct Node* packetQueue = NULL;

    for (int i = 0; i < total; i++) {
        struct Packet p;
        p.sequence = i;
        p.total_frames = total;
        strcpy(p.generator, "1101");

        printf("Enter string data payload for Frame %d: ", i);
        fgets(p.data, BUFFER_SIZE, stdin);
        p.data[strcspn(p.data, "\n")] = '\0'; // Remove dangling newline

        p.test_case = 0; // Default initialization
        strcpy(p.crc, "");
        appendNode(&packetQueue, p);
    }

    struct Node* current = packetQueue;
    while (current != NULL) {
        int choice = 0;
        printf("\n--- Transmission Options for Frame %d ('%s') ---\n", current->packet.sequence, current->packet.data);
        printf("0 -> Send Normally (Valid CRC)\n");
        printf("1 -> Force Frame Drop Simulation (Triggers Timeout)\n");
        printf("2 -> Induce CRC Bit Error Corruption (Triggers Server NACK)\n");
        printf("Select action (0-2): ");
        scanf("%d", &choice);
        getchar();

        // Update the packet based on runtime selection
        if (choice == 1) current->packet.test_case = 3;
        else if (choice == 2) current->packet.test_case = 4;
        else current->packet.test_case = 0;

        generateCRC(&(current->packet));
        if (current->packet.test_case == 4) {
            current->packet.crc[0] = (current->packet.crc[0] == '1') ? '0' : '1'; // Corrupt a bit
        }

        if (current->packet.test_case == 3) {
            printf("[Client] Simulating missing packet link: Sending Frame %d (Dropped configuration)...\n", current->packet.sequence);
        } else if (current->packet.test_case == 4) {
            printf("[Client] Injecting noise: Sending Frame %d with Corrupted CRC bits...\n", current->packet.sequence);
        } else {
            printf("[Client] Sending Frame %d cleanly: '%s'\n", current->packet.sequence, current->packet.data);
        }

        send(cfd, &(current->packet), sizeof(struct Packet), 0);

        // 2-Second Non-Blocking Timeout Checker (Case 3)
        fd_set readfds;
        struct timeval tv = {2, 0};
        FD_ZERO(&readfds);
        FD_SET(cfd, &readfds);

        int activity = select(cfd + 1, &readfds, NULL, NULL, &tv);

        if (activity > 0) {
            memset(response, 0, sizeof(response));
            int bytes = recv(cfd, response, sizeof(response) - 1, 0);
            if (bytes <= 0) break;

            printf("[Client] Server Feedback Received: %s\n", response);

            int ack_seq;
            if (sscanf(response, "ACK %d", &ack_seq) == 1) {
                if (ack_seq == current->packet.sequence) {
                    printf("[Client] Successful ACK match! Freeing node and moving to next frame.\n");
                    struct Node* temp = current;
                    current = current->next;
                    free(temp);
                }
            } else if (strncmp(response, "NACK", 4) == 0) {
                printf("[Client] NACK received due to CRC error! Staying on current node for automatic repair resend.\n");
            }
        } else {
            printf("[Timeout Alert] No response back within 2 seconds. Staying on Frame %d to resend.\n", current->packet.sequence);
        }
    }

    printf("\n[System] All items in list successfully cleared and safely sent.\n");
    close(cfd);
    return 0;
}
