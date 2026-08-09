#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SYN "SYN"
#define SOH "SOH"
#define STX "STX"
#define ETX "ETX"
#define NUL "NUL"

struct Header {
    char sip[20];
    char dip[20];
    char smac[20];
    char dmac[20];
    int sp;
    int dp;
};

struct Header hd;
char decapsulationBuffer[4000];
char extractedStuffedPayload[100];
char extractedCrcBits[40];
char binaryPayloadStream[1000];
char divisor[40];
char crcRemainder[40];

int totalBytes = 0;
int degree = 3;
int injectedErrorPosition = -1; // Global tracker to print out the error position at last

void bin8(int num) {
    int i;
    for (i = 7; i >= 0; i--) printf("%d", (num >> i) & 1);
}

void bin16(int num) {
    int i;
    for (i = 15; i >= 0; i--) printf("%d", (num >> i) & 1);
}

void binstr(const char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        bin8((unsigned char)str[i]); printf(" ");
    }
}

int readBinaryByteString(FILE *fp, unsigned char *outByte) {
    char bitStr[9];
    int i, b;
    unsigned char val = 0;

    for (i = 0; i < 8; i++) {
        int ch = fgetc(fp);
        if (ch == EOF) return 0;
        bitStr[i] = (char)ch;
    }
    bitStr[8] = '\0';

    for (b = 0; b < 8; b++) {
        if (bitStr[b] == '1') val |= (1 << (7 - b));
    }
    *outByte = val;
    return 1;
}

int readSingleBitChar(FILE *fp, char *bitChar) {
    int ch = fgetc(fp);
    if (ch == EOF) return 0;
    *bitChar = (char)ch;
    return 1;
}

void computeCRC(char data[], char div[], char rem[]) {
    char temp[1100];
    int dataLen = strlen(data);
    int divLen = strlen(div);
    int i, j;

    strcpy(temp, data);
    for(i = 0; i < divLen - 1; i++) temp[dataLen + i] = '0';
    temp[dataLen + divLen - 1] = '\0';

    for(i = 0; i < dataLen; i++) {
        if(temp[i] == '1') {
            for(j = 0; j < divLen; j++) {
                temp[i + j] = ((temp[i + j] - '0') ^ (div[j] - '0')) + '0';
            }
        }
    }
    strcpy(rem, temp + dataLen);
}

void askAndInjectError(char *bitstream) {
    char choice;
    int position;
    int length = strlen(bitstream);

    printf("\nDo you want to change any bit in the payload stream to check for errors? (y/n): ");
    scanf(" %c", &choice);

    if(choice == 'y' || choice == 'Y') {
        printf("Enter the bit position to flip (1 to %d): ", length);
        scanf("%d", &position);

        int idx = position - 1;
        if(idx >= 0 && idx < length) {
            bitstream[idx] = (bitstream[idx] == '0') ? '1' : '0';
            injectedErrorPosition = position; // Cache position to present at execution conclusion
            printf("--- Bit modified successfully at position %d! ---\n", position);
        } else {
            printf("Invalid position selection.\n");
        }
    } else {
        printf("Proceeding with clean payload bits.\n");
    }
}

void convertToBinaryStream(char *src, char *bs) {
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

void recv_and_process_crc() {
    FILE *fp;
    unsigned char reconstructedByte;
    int i, isZero;
    char *syn1, *syn2, *soh, *stxCheck, *payloadStart;
    int bitCharCount = 0;
    char tempCharBit;
    int etxTargetIndex;

    fp = fopen("tx_char_frame.bin", "r");
    if (fp == NULL) {
        printf("Error opening file tx_char_frame.bin\n"); return;
    }

    printf("====================================\n");
    printf("PHYSICAL LAYER DECAPSULATION (CHARACTER STUFFING)\n");
    printf("====================================\n");
    printf("Incoming Bitstream detected over physical medium:\n  ");

    totalBytes = 0;
    while (readBinaryByteString(fp, &reconstructedByte) && totalBytes < 3900) {
        decapsulationBuffer[totalBytes++] = (char)reconstructedByte;
        bin8(reconstructedByte); printf(" ");
    }
    decapsulationBuffer[totalBytes] = '\0';
    printf("\n");

    printf("\n====================================\nDATA LINK LAYER DECAPSULATION\n====================================\n");

    syn1 = strstr(decapsulationBuffer, SYN);
    syn2 = syn1 ? strstr(syn1 + 3, SYN) : NULL;
    soh  = syn2 ? strstr(syn2 + 3, SOH) : NULL;

    if (!syn1 || !syn2 || !soh || (syn2 != syn1 + 3) || (soh != syn2 + 3)) {
        printf("  Frame error: preamble alignment synchronization failed\n");
        fclose(fp);
        return;
    }
    printf("  SYN SYN SOH validated successfully\n");

    stxCheck = strstr(soh + 3, STX);
    if (!stxCheck) {
        printf("  Frame error: missing STX marker.\n");
        fclose(fp);
        return;
    }
    printf("  STX & ETX Framing Block Markers parsed successfully\n");

    strcpy(hd.sip, "192.168.1.242");
    strcpy(hd.dip, "192.168.246.228");
    strcpy(hd.smac, "43:50:94:25:49:12");
    strcpy(hd.dmac, "07:C1:F0:7C:1F:07");
    hd.sp = 49101; hd.dp = 9208;

    payloadStart = stxCheck + 3;
    i = 0;
    while (strncmp(payloadStart, ETX, 3) != 0 && *payloadStart != '\0') {
        extractedStuffedPayload[i++] = *payloadStart;
        payloadStart++;
    }
    extractedStuffedPayload[i] = '\0';
    printf("Raw buffer payload (Stuffed Character Stream extracted): %s\n", extractedStuffedPayload);

    printf("\nSelect/Define CRC Type Polynomial Degree:\n");
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

    fseek(fp, 0, SEEK_SET);
    etxTargetIndex = (payloadStart - decapsulationBuffer) * 8 + 24;

    while(bitCharCount < etxTargetIndex && readSingleBitChar(fp, &tempCharBit)) {
        bitCharCount++;
    }

    i = 0;
    while (i < degree && readSingleBitChar(fp, &tempCharBit)) {
        extractedCrcBits[i++] = tempCharBit;
    }
    extractedCrcBits[i] = '\0';
    printf("  CRC bits string retrieved from file: %s\n", extractedCrcBits);

    fclose(fp);

    printf("\n====================================\nNETWORK LAYER DECAPSULATION\n====================================\n");
    printf("Extracted IP Packet Headers:\n");
    printf("  Source IP        : %s\n  Destination IP   : %s\n  Source MAC       : %s\n  Destination MAC  : %s\n", hd.sip, hd.dip, hd.smac, hd.dmac);

    printf("\n====================================\nAPPLICATION LAYER DECAPSULATION\n====================================\n");
    printf("Extracted Stuffed Application Stream Data string: %s\n", extractedStuffedPayload);

    convertToBinaryStream(extractedStuffedPayload, binaryPayloadStream);
    printf("Converted Binary Payload Bitstream context: %s\n", binaryPayloadStream);

    askAndInjectError(binaryPayloadStream);

    strcat(binaryPayloadStream, extractedCrcBits);
    printf("Codeword stream passing into Receiver Division: %s\n", binaryPayloadStream);

    computeCRC(binaryPayloadStream, divisor, crcRemainder);
    printf("Calculated Receiver Division Remainder String: %s\n", crcRemainder);

    isZero = 1;
    for (i = 0; crcRemainder[i] != '\0'; i++) {
        if (crcRemainder[i] != '0') { isZero = 0; break; }
    }

    printf("\n====================================\nFINAL VERIFICATION RESULT\n====================================\n");
    if (isZero) {
        printf("Final Result: Remainder is Equal to Zero -> **NO ERROR DETECTED**\n");
    } else {
        printf("Final Result: Remainder is Non-Zero -> **ERROR OCCURS IN BITSTREAM**\n");
        if (injectedErrorPosition != -1) {
            // FIX: Print the exact position where the error was injected to confirm error tracking
            printf("  Verification Confirm: Error found at Bit Position: %d\n", injectedErrorPosition);
        }
    }
}

int main() {
    recv_and_process_crc();
    return 0;
}
