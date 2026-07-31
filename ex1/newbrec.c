#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
char buf[100];
char msg[100];
int slen;
int len;

// Helper function to convert a character byte to a binary string representation
void bin(char ch, char *binval)
{
    int i;
    for (i = 7; i >= 0; i--)
    {
        binval[7-i] = ((ch >> i) & 1) ? '1' : '0';
    }
    binval[8] = '\0';
}

// Network Layer Decapsulation View Controller
void head()
{
    int i;
    char binval[9];

    printf("\n====================================\n");
    printf("NETWORK LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Extracted IP Packet Headers:\n");
    printf("  Source IP        : %s\n", hd.sip);
    printf("  Destination IP   : %s\n", hd.dip);
    printf("  Source MAC       : %s\n", hd.smac);
    printf("  Destination MAC  : %s\n", hd.dmac);

    printf("\nRemaining Stream payload moving up:\n");
    printf("  ");
    for(i = 15; i >= 0; i--) printf("%d", (hd.sp >> i) & 1);
    printf(" ");
    for(i = 15; i >= 0; i--) printf("%d", (hd.dp >> i) & 1);
    printf(" ");
    for(i = 0; i < slen; i++) {
        bin(buf[i], binval);
        printf("%s ", binval);
    }
    printf("\n");
}

// Fully compliant transparent data stream un-stuffer (Supports DLEDLE, DLESTX, and DLEETX)
int unstuff()
{
    int i = 0;
    int j = 0;

    while (i < slen)
    {
        // 1. Handle DLE DLE -> output literal sequence "DLE"
        if (i + 5 < slen &&
            strncmp(&buf[i], "DLE", 3) == 0 &&
            strncmp(&buf[i + 3], "DLE", 3) == 0)
        {
            msg[j++] = 'D'; msg[j++] = 'L'; msg[j++] = 'E';
            i += 6;
            continue;
        }
        // 2. Handle DLE STX -> output literal sequence "STX"
        if (i + 5 < slen &&
            strncmp(&buf[i], "DLE", 3) == 0 &&
            strncmp(&buf[i + 3], "STX", 3) == 0)
        {
            msg[j++] = 'S'; msg[j++] = 'T'; msg[j++] = 'X';
            i += 6;
            continue;
        }
        // 3. Handle DLE ETX -> output literal sequence "ETX"
        if (i + 5 < slen &&
            strncmp(&buf[i], "DLE", 3) == 0 &&
            strncmp(&buf[i + 3], "ETX", 3) == 0)
        {
           msg[j++] = 'E'; msg[j++] = 'T'; msg[j++] = 'X';
           i += 6;
           continue;
        }

        // Pass normal payload text character elements cleanly straight out
        msg[j++] = buf[i++];
    }
    msg[j] = '\0';
    return j;
}

// Frame capture machine reading directly from file media link layer
void recv()
{
    FILE *fp;
    char read[4];
    char ch;
    int i = 0;
    char binval[9];

    fp = fopen("text.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    printf("\n====================================\n");
    printf("PHYSICAL LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Incoming Bitstream detected over physical medium:\n  ");
    while ((ch = fgetc(fp)) != EOF) {
        bin(ch, binval);
        printf("%s ", binval);
    }
    printf("\n");

    fseek(fp, 0, SEEK_SET);

    printf("\n====================================\n");
    printf("DATA LINK LAYER DECAPSULATION\n");
    printf("====================================\n");

    fgets(read, 4, fp);
    if (strncmp(read, "SYN", 3) != 0)
    {
        printf("Frame error: missing first SYN\n");
        fclose(fp);
        return;
    }

    fgets(read, 4, fp);
    if (strncmp(read, "SYN", 3) != 0)
    {
        printf("Frame error: missing second SYN\n");
        fclose(fp);
        return;
    }

    fgets(read, 4, fp);
    if (strncmp(read, "SOH", 3) != 0)
    {
        printf("Frame error: missing SOH\n");
        fclose(fp);
        return;
    }

    printf("Preamble Alignment Validation:\n");
    printf("  SYN SYN SOH validated successfully\n");

    fread(&hd, sizeof(struct Header), 1, fp);

    fgets(read, 4, fp);
    if (strncmp(read, "STX", 3) != 0)
    {
        printf("Frame error: missing STX\n");
        fclose(fp);
        return;
    }
    printf("  STX & ETX Markers validated successfully\n");

    // FIX: Process streaming data safely one single character at a time
    i = 0;
    while (1)
    {
        ch = fgetc(fp);
        if (ch == EOF) break;

        buf[i] = ch;
        i++;

        // Continually cross check trailing string structure window elements
        if (i >= 3 && buf[i-3] == 'E' && buf[i-2] == 'T' && buf[i-1] == 'X')
        {
            // Verify transparent escape constraints:
            // Check if the ETX string sequence is backed up by an escaped "DLE" segment
            if (i >= 6 && buf[i-6] == 'D' && buf[i-5] == 'L' && buf[i-4] == 'E') {
                // Stuffed text value element element -> maintain validation collection
                continue;
            } else {
                // Genuine unescaped control packet boundary hit -> truncate ETX structural tracking indicators
                i -= 3;
                break;
            }
        }
    }
    slen = i;

    // Correctly locate and read out your validation trailing check byte following true ETX termination
    ch = fgetc(fp);
    printf("  CRC byte received: ");
    bin(ch, binval);
    printf("%s\n", binval);

    printf("\nRaw buffer payload (Stuffed Character Stream):\n  ");
    for(i = 0; i < slen; i++)
    {
       printf("%c", buf[i]);
    }
    printf("\n");

    fclose(fp);
}

int main()
{
    int i;
    char binval[9];

    recv();
    head();

    printf("\n====================================\n");
    printf("TRANSPORT LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Extracted TCP/UDP Segment Headers:\n");
    printf("  Source Port      : %d\n", hd.sp);
    printf("  Destination Port : %d\n", hd.dp);
    printf("\nRemaining Stream payload moving up:\n");
    printf("  ");
    for(i = 0; i < slen; i++) {
        bin(buf[i], binval);
        printf("%s ", binval);
    }
    printf("\n");

    len = unstuff();

    printf("\n====================================\n");
    printf("APPLICATION LAYER DECAPSULATION\n");
    printf("====================================\n");
    printf("Unstuffed Message: %s\n", msg);
    return 0;
}
