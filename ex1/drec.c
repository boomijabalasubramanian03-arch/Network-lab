#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SYN "SYN"
#define SOH "SOH"
#define NUL "NUL"

struct Header
{
    int sp;
    int dp;
    char sip[20];
    char dip[20];
    char smac[20];
    char dmac[20];
};

struct Header hd;
char msg[100];
int len;

void bin8(int num)
{
    int i;
    for (i = 7; i >= 0; i--)
    {
        printf("%d", (num >> i) & 1);
    }
}

void bin16(int num)
{
    int i;
    for (i = 15; i >= 0; i--)
    {
        printf("%d", (num >> i) & 1);
    }
}

void binstr(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        bin8((unsigned char)str[i]);
        printf(" ");
    }
}

void head()
{
    printf("  Source IP        : %s\n", hd.sip);
    printf("  Destination IP   : %s\n", hd.dip);
    printf("  Source MAC       : %s\n", hd.smac);
    printf("  Destination MAC  : %s\n", hd.dmac);
}

void recv()
{
    FILE *fp;
    char fileBuffer[4000];
    int totalBytes = 0;
    int ch;
    int i;

    fp = fopen("tx_frame.bin", "rb");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    printf("====================================\n");
    printf("PHYSICAL LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Incoming Bitstream detected over physical medium:\n  ");

    while ((ch = fgetc(fp)) != EOF && totalBytes < 4000)
    {
        fileBuffer[totalBytes++] = (char)ch;
        bin8(ch);
        printf(" ");
    }
    printf("\n");
    fclose(fp);

    printf("\n====================================\n");
    printf("DATA LINK LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Preamble Alignment Validation:\n");

    char *syn1 = strstr(fileBuffer, SYN);
    char *syn2 = syn1 ? strstr(syn1 + 3, SYN) : NULL;
    char *soh  = syn2 ? strstr(syn2 + 3, SOH) : NULL;

    if (!syn1 || !syn2 || !soh || (syn2 != syn1 + 3) || (soh != syn2 + 3))
    {
        printf("  Frame error: preamble alignment synchronization failed\n");
        return;
    }
    printf("  SYN SYN SOH validated successfully\n");

    char *lengthPointer = soh + 3;
    int lenHigh = (unsigned char)lengthPointer[0];
    int lenLow  = (unsigned char)lengthPointer[1];
    len = (lenHigh << 8) | lenLow;

    printf("Byte Count System Tracking Check:\n");
    printf("  Extracted Payload Length count: %d bytes (%d bits)\n", len, len * 8);

    char *headerPointer = lengthPointer + 2;


    memcpy(hd.sip, headerPointer, 20);
    hd.sip[19] = '\0';
    headerPointer += 20;

    memcpy(hd.dip, headerPointer, 20);
    hd.dip[19] = '\0';
    headerPointer += 20;

    memcpy(hd.smac, headerPointer, 20);
    hd.smac[19] = '\0';
    headerPointer += 20;

    memcpy(hd.dmac, headerPointer, 20);
    hd.dmac[19] = '\0';
    headerPointer += 20;


    hd.sp = 49101;
    hd.dp = 9208;

    char *payloadPointer = headerPointer;
    for (i = 0; i < len; i++)
    {
        msg[i] = payloadPointer[i];
    }
    msg[len] = '\0';

    char *nulCheck = strstr(payloadPointer + len, NUL);
    if (nulCheck)
    {
        printf("  NUL Boundary Validation: Success\n");
    }
    else
    {
        printf("  Warning: NUL Boundary Validation Mismatch\n");
    }

    printf("\n====================================\n");
    printf("NETWORK LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Extracted IP Packet Headers:\n");
    head();

    printf("\nRemaining Stream payload moving up:\n  ");
    bin16(hd.sp); printf(" ");
    bin16(hd.dp); printf(" ");
    binstr(msg);
    printf("\n");

    printf("\n====================================\n");
    printf("TRANSPORT LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Extracted TCP/UDP Segment Headers:\n");
    printf("  Source Port      : %d\n", hd.sp);
    printf("  Destination Port : %d\n", hd.dp);

    printf("\nRemaining Stream payload moving up:\n  ");
    binstr(msg);
    printf("\n");

    printf("\n====================================\n");
    printf("APPLICATION LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Extracted Message: %s\n", msg);
}

int main()
{
    recv();
    return 0;
}
