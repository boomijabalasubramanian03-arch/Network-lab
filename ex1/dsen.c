#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SYN "SYN"
#define SOH "SOH"
#define NUL "NUL"

struct Node {
    char url[50];
    char ip[20];
    char mac[20];
    int port;
};

struct Node table[2] = {
    {"www.google.com", "192.168.1.242", "43:50:94:25:49:12", 49101},
    {"www.email.com",  "192.168.246.228", "07:C1:F0:7C:1F:07", 9208}
};

char srcUrl[50], destUrl[50];
char srcIp[20], destIp[20];
char srcMac[20], destMac[20];
int srcPort = 0, destPort = 0;

char payload[100];
int payloadLen = 0;
int packetSize = 0;
int frameSize = 0;

void printByte(int num) {
    int i;
    for (i = 7; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
    }
}

void printString(const char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        printByte((unsigned char)str[i]);
        printf(" ");
    }
}

void printInt(unsigned int value, int totalBits) {
    int i;
    for (i = totalBits - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
}

void lookupAddresses() {
    int i;
    for (i = 0; i < 2; i++) {
        if (strcmp(srcUrl, table[i].url) == 0) {
            strcpy(srcIp, table[i].ip);
            strcpy(srcMac, table[i].mac);
            srcPort = table[i].port;
        }
        if (strcmp(destUrl, table[i].url) == 0) {
            strcpy(destIp, table[i].ip);
            strcpy(destMac, table[i].mac);
            destPort = table[i].port;
        }
    }
    printf("Mapped successfully.\n");
}

void processLayers() {
    int totalPayloadBits = payloadLen * 8;

    printf("\n====================================\nAPPLICATION LAYER\n====================================\n");
    printf("Application Data : %s\n", payload);
    printf("Output stream (Data bits): ");
    printString(payload);
    printf("\n");

    printf("\n====================================\nTRANSPORT LAYER\n====================================\n");
    printf("Added Segment Headers:\n");
    printf("  Source Port bits     : "); printInt(srcPort, 16); printf("\n");
    printf("  Destination Port bits: "); printInt(destPort, 16); printf("\n");

    printf("\n[Appended Output stream] -> Segment (Ports + Data):\n");
    printInt(srcPort, 16); printf(" ");
    printInt(destPort, 16); printf(" ");
    printString(payload);
    printf("\n");

    printf("\n====================================\nNETWORK LAYER\n====================================\n");
    printf("Added IP Packet Headers:\n");
    printf("  Source IP bits     : "); printString(srcIp); printf("\n");
    printf("  Destination IP bits: "); printString(destIp); printf("\n");

    int headerBits = 32;
    int totalNetworkBits = headerBits + totalPayloadBits;
    int numPackets = (totalNetworkBits + packetSize - 1) / packetSize;

    int *bitStream = (int *)calloc(2000, sizeof(int));
    int bitIndex = 0, i, j;

    for (i = 15; i >= 0; i--) bitStream[bitIndex++] = (srcPort >> i) & 1;
    for (i = 15; i >= 0; i--) bitStream[bitIndex++] = (destPort >> i) & 1;
    for (i = 0; i < payloadLen; i++) {
        for (j = 7; j >= 0; j--) {
            bitStream[bitIndex++] = (payload[i] >> j) & 1;
        }
    }

    int offset = 0;
    for (i = 0; i < numPackets; i++) {
        int endBit = offset + packetSize - 1;

        printf("\nPACKET %d (Bits %d to %d)\n", i + 1, offset, endBit);
        printf("[Appended Output stream] -> Packet (IPs + Ports + Data Sub-slice):\n");
        printString(srcIp); printf(" ");
        printString(destIp); printf("  ");

        for (j = offset; j <= endBit; j++) {
            if (j < totalNetworkBits) {
                printf("%d", bitStream[j]);
            } else {
                printf("0");
            }
            if ((j - offset + 1) % 8 == 0 && j != endBit) printf(" ");
        }
        printf("\n");
        offset += packetSize;
    }

    printf("\n====================================\nDATA LINK LAYER (BYTE-COUNT SYSTEM)\n====================================\n");
    printf("[Byte Count System Tracking Check]:\n");
    printf("  Original Message String : %s\n", payload);
    printf("  Payload Byte Count      : %d bytes (%d bits)\n", payloadLen, totalPayloadBits);

    for (i = 0; i < numPackets; i++) {
        int bitsRemaining = packetSize;
        int frameCount = 1;

        printf("\n------------------------------------\nPACKET %d FRAME SEGMENTATION DETAILS\n------------------------------------\n", i + 1);
        printf("Added Frame MAC Headers:\n");
        printf("  Source MAC bits     : "); printString(srcMac); printf("\n");
        printf("  Destination MAC bits: "); printString(destMac); printf("\n");
        printf("\n[Appended Output stream Processed inside Frame Windows]:\n");

        int packetPointer = i * packetSize;
        while (bitsRemaining > 0) {
            int currentFrameSize = (bitsRemaining > frameSize) ? frameSize : bitsRemaining;
            int startFrameBit = (frameCount - 1) * frameSize;
            int endFrameBit = startFrameBit + frameSize - 1;

            printf("  Frame %d (Bits %d to %d) -> Full Frame bits:\n      ", frameCount, startFrameBit, endFrameBit);

            for (j = 0; j < frameSize; j++) {
                int globalPos = packetPointer + (frameCount - 1) * frameSize + j;
                if (globalPos < totalNetworkBits && j < currentFrameSize) {
                    printf("%d", bitStream[globalPos]);
                } else {
                    printf("0");
                }
            }
            printf("\n");
            bitsRemaining -= frameSize;
            frameCount++;
        }
    }
    free(bitStream);
}

void writeBinaryFile() {
    FILE *fp = fopen("tx_frame.bin", "wb");
    if (fp == NULL) return;

    fputs(SYN, fp);
    fputs(SYN, fp);
    fputs(SOH, fp);

    char mask = ~'\0';
    fputc((payloadLen >> 8) & mask, fp);
    fputc(payloadLen & mask, fp);

    fwrite(srcIp, 1, 20, fp);
    fwrite(destIp, 1, 20, fp);
    fwrite(srcMac, 1, 20, fp);
    fwrite(destMac, 1, 20, fp);
    fwrite(payload, 1, payloadLen, fp);

    fputs(NUL, fp);
    fclose(fp);
    printf("\nFrame written to tx_frame.bin using pure control token syntax.\n");
}

void readAndPrintBinaryFile() {
    FILE *fp = fopen("tx_frame.bin", "rb");
    if (fp == NULL) return;

    printf("\n--- Final File Frame in Binary ---\n");
    int ch = fgetc(fp);
    while (ch != EOF) {
        printByte(ch);
        printf(" ");
        ch = fgetc(fp);
    }
    printf("\n");
    fclose(fp);
}

int main() {
    printf("Enter Source URL : ");
    scanf("%s", srcUrl);
    printf("Enter Destination URL : ");
    scanf("%s", destUrl);

    printf("\nSTORED VALUES IN STRUCTURE TABLE\n");
    lookupAddresses();

    printf("Enter Payload (no spaces): ");
    scanf("%s", payload);
    payloadLen = strlen(payload);

    printf("Enter Packet Size (in bits): ");
    scanf("%d", &packetSize);
    printf("Enter Frame Size (in bits): ");
    scanf("%d", &frameSize);

    processLayers();
    writeBinaryFile();
    readAndPrintBinaryFile();

    return 0;
}
