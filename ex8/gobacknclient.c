#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 3

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
    for (int i = 0; i < genLength - 1; i++) strcat(temp, "0");

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

    if (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) exit(1);
    printf("--- Connected to Go-Back-N Server ---\n\n");

    printf("Enter total number of frames to write into sliding queue: ");
    scanf("%d", &total);
    getchar();

    struct Node* queue = NULL;
    for (int i = 0; i < total; i++) {
        struct Packet p;
        p.sequence = i;
        p.total_frames = total;
        strcpy(p.generator, "1101");

        printf("Enter string data payload for Frame %d: ", i);
        fgets(p.data, BUFFER_SIZE, stdin);
        p.data[strcspn(p.data, "\n")] = '\0';

        p.test_case = 0;
        strcpy(p.crc, "");
        appendNode(&queue, p);
    }

    struct Node* window_start = queue;
    struct Node* next_to_send = queue;

    while (window_start != NULL) {
        int sent_count = 0;
        struct Node* temp = window_start;
        while (temp != next_to_send) {
            sent_count++;
            temp = temp->next;
        }

        // Pipelined Window Block Generation loop
        while (next_to_send != NULL && sent_count < WINDOW_SIZE) {
            int choice = 0;
            printf("\n[Pipeline Prompt] Configure Transmission for Frame %d ('%s'):\n", next_to_send->packet.sequence, next_to_send->packet.data);
            printf("0 -> Send Normally\n1 -> Drop Simulation\n2 -> Inject CRC Error\nChoice: ");
            scanf("%d", &choice);
            getchar();

            if (choice == 1) next_to_send->packet.test_case = 3;
            else if (choice == 2) next_to_send->packet.test_case = 4;
            else next_to_send->packet.test_case = 0;

            generateCRC(&(next_to_send->packet));
            if (next_to_send->packet.test_case == 4) {
                next_to_send->packet.crc[0] = (next_to_send->packet.crc[0] == '1') ? '0' : '1';
            }

            printf("[GBN Client] Sending Frame %d...\n", next_to_send->packet.sequence);
            send(cfd, &(next_to_send->packet), sizeof(struct Packet), 0);

            next_to_send = next_to_send->next;
            sent_count++;
        }

        // Non-blocking timer check
        fd_set readfds;
        struct timeval tv = {2, 0};
        FD_ZERO(&readfds);
        FD_SET(cfd, &readfds);

        int activity = select(cfd + 1, &readfds, NULL, NULL, &tv);
        if (activity > 0) {
            memset(response, 0, sizeof(response));
            recv(cfd, response, sizeof(response) - 1, 0);

            int ack_seq;
            sscanf(response, "ACK %d", &ack_seq);
            printf("\n[GBN Client] Receiver Cumulative Feedback: %s\n", response);

            if (ack_seq >= window_start->packet.sequence) {
                while (window_start != NULL && window_start->packet.sequence <= ack_seq) {
                    struct Node* to_free = window_start;
                    window_start = window_start->next;
                    free(to_free);
                }
                printf("ACK matched. Sliding window boundaries forward!\n");
            }
        } else {
            // Case 3 Timeout Event: Go back to window base pointer boundary
            printf("\n[Timeout] Missing expected ACK segment block. Rewinding pipeline to unacknowledged Frame %d!\n", window_start->packet.sequence);
            next_to_send = window_start;
        }
    }

    printf("\n[System] Sliding list window empty. All data transferred successfully.\n");
    close(cfd);
    return 0;
}
