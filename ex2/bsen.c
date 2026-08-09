#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SYN "SYN"
#define SOH "SOH"
#define STX "STX"
#define ETX "ETX"
#define NUL "NUL"

struct Node {
    char url[50];
    char ip[20];
    char mac[20];
    int port;
};

struct Node table[2] = {
    {"://google.com", "192.168.1.242", "43:50:94:25:49:12", 49101},
    {"://email.com",  "192.168.246.228", "07:C1:F0:7C:1F:07", 9208}
};

char srcUrl[50], destUrl[50];
char srcIp[20], destIp[20];
char srcMac[20], destMac[20];
int srcPort = 0, destPort = 0;

char payload[100];
char sPayload[200];
char binval[2000];
char crcBits[40];
char divisor[40];

int pLen = 0;
int sLen = 0;
int pSize = 0;
int fSize = 0;
int degree = 3;

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

int ByteStuffing(char *src, char *dst) {
    int i = 0, j = 0;
    int sl = strlen(src);
    while (i < sl) {
        if (strncmp(&src[i], "STX", 3) == 0) {
            strcpy(&dst[j], "DLESTX");
            j += 6; i += 2;
        } else if (strncmp(&src[i], "ETX", 3) == 0) {
            strcpy(&dst[j], "DLEETX");
            j += 6; i += 2;
        } else if (strncmp(&src[i], "DLE", 3) == 0) {
            strcpy(&dst[j], "DLEDLE");
            j += 6; i += 2;
        } else {
            dst[j++] = src[i];
        }
        i++;
    }
    dst[j] = '\0';
    return j;
}

void convertbin(char *src, char *bs) {
    int i = 0, b;
    bs[0] = '\0';
    while (src[i] != '\0') {
        for (b = 7; b >= 0; b--) {
            char bit = ((src[i] >> b) & 1) ? '1' : '0';
            sprintf(&bs[strlen(bs)], "%c", bit);
        }
        i++;
    }
}

void computeCRC(char data[], char div[], char remainder[]) {
    char temp[2500];
    int dataLen = strlen(data);
    int divLen = strlen(div);
    int i, j;

    strcpy(temp, data);
    for(i = 0; i < divLen - 1; i++) {
        temp[dataLen + i] = '0';
    }
    temp[dataLen + divLen - 1] = '\0';

    for(i = 0; i < dataLen; i++) {
        if(temp[i] == '1') {
             for(j = 0; j < divLen; j++) {
                temp[i + j] = ((temp[i + j] - '0') ^ (div[j] - '0')) + '0';
            }
        }
    }
    strcpy(remainder, temp + dataLen);
}

void processLayers() {
    int totalBits = pLen * 8;
    int headerBits = 32;
    int totalNetworkBits = headerBits + totalBits;
    int numPackets = (totalNetworkBits + pSize - 1) / pSize;
    int *bitStream = (int *)calloc(2000, sizeof(int));
    int bitIndex = 0, i, j, offset;

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

    for (i = 15; i >= 0; i--) bitStream[bitIndex++] = (srcPort >> i) & 1;
    for (i = 15; i >= 0; i--) bitStream[bitIndex++] = (destPort >> i) & 1;
    for (i = 0; i < pLen; i++) {
        for (j = 7; j >= 0; j--) {
            bitStream[bitIndex++] = (payload[i] >> j) & 1;
        }
    }

    offset = 0;
    for (i = 0; i < numPackets; i++) {
        int endBit = offset + pSize - 1;
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
        offset += pSize;
    }

    printf("\n====================================\nDATA LINK LAYER (CHARACTER STUFFING METHOD)\n====================================\n");
    sLen = ByteStuffing(payload, sPayload);
    printf("[Character Transparency Processing via Character Stuffing Tracing]:\n");
    printf("  Original Message String : %s\n", payload);
    printf("  Stuffed Output Stream   : %s\n\n", sPayload);

    free(bitStream);
}

void writebin() {
    FILE *fp = fopen("tx_char_frame.bin", "wb");
    int i, j, b;
    const char *preamble[3];
    char *headers[4];

    preamble[0] = SYN; preamble[1] = SYN; preamble[2] = SOH;
    headers[0] = srcIp; headers[1] = destIp; headers[2] = srcMac; headers[3] = destMac;

    if (fp == NULL) return;

    // 1. Write SYN SYN SOH
    for (i = 0; i < 3; i++) {
        for (j = 0; preamble[i][j] != '\0'; j++) {
            for (b = 7; b >= 0; b--) {
                fputc(((preamble[i][j] >> b) & 1) ? '1' : '0', fp);
            }
        }
    }

    // 2. Write IP and MAC Address Header structures
    for (i = 0; i < 4; i++) {
        for (j = 0; headers[i][j] != '\0'; j++) {
            for (b = 7; b >= 0; b--) {
                fputc(((headers[i][j] >> b) & 1) ? '1' : '0', fp);
            }
        }
    }

    // 3. Write opening framing control tag "STX"
    for (i = 0; STX[i] != '\0'; i++) {
        for (b = 7; b >= 0; b--) fputc(((STX[i] >> b) & 1) ? '1' : '0', fp);
    }

    // 4. Write the stuffed payload character components
    for (i = 0; i < sLen; i++) {
        for (b = 7; b >= 0; b--) {
            fputc(((sPayload[i] >> b) & 1) ? '1' : '0', fp);
        }
    }

    // 5. Write closing framing control tag "ETX"
    for (i = 0; ETX[i] != '\0'; i++) {
        for (b = 7; b >= 0; b--) fputc(((ETX[i] >> b) & 1) ? '1' : '0', fp);
    }

    // 6. FIX: Write CRC Bits explicitly as individual character digits ('1' and '0') to file stream
    for (i = 0; crcBits[i] != '\0'; i++) {
        fputc(crcBits[i], fp);
    }

    // 7. Write terminating protocol frame boundary tag "NUL"
    for (i = 0; NUL[i] != '\0'; i++) {
        for (b = 7; b >= 0; b--) fputc(((NUL[i] >> b) & 1) ? '1' : '0', fp);
    }

    fclose(fp);
    printf("\nCHARACTER STUFFED frame written safely to tx_char_frame.bin.\n");
}

int main() {
    int i;
    printf("Enter Source URL : ");
    scanf("%s", srcUrl);
    printf("Enter Destination URL : ");
    scanf("%s", destUrl);

    printf("\nSTORED VALUES IN STRUCTURE TABLE\n");
    lookupAddresses();

    printf("Enter Payload (no spaces): ");
    scanf("%s", payload);
    pLen = strlen(payload);

    printf("Enter Packet Size (in bits): ");
    scanf("%d", &pSize);
    printf("Enter Frame Size (in bits): ");
    scanf("%d", &fSize);

    processLayers();

    printf("\nSelect Sender CRC Type Polynomial Degree:\n");
    printf("  Enter highest degree of your preferred polynomial (3, 8, 10, 16, 32): ");
    scanf("%d", &degree);

    switch(degree) {
        case 3:  strcpy(divisor, "1011"); break;
        case 8:  strcpy(divisor, "100000111"); break;
        case 10: strcpy(divisor, "11000110101"); break;
        case 16: strcpy(divisor, "10001000000100001"); break;
        case 32: strcpy(divisor, "100000100110000010001110110110111"); break;
        default:
            divisor[0] = '1';
            for(i = 1; i < degree - 1; i++) divisor[i] = '0';
            divisor[degree - 1] = '1'; divisor[degree] = '1'; divisor[degree + 1] = '\0';
            break;
    }
    printf("Active Binary Polynomial Divisor Target String: %s\n", divisor);

    convertbin(sPayload, binval);
    computeCRC(binval, divisor, crcBits);
    printf("Calculated Sender Side CRC bits: %s\n", crcBits);

    writebin();
    return 0;
}
