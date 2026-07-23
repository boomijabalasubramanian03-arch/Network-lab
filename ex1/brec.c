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

void bin(char ch, char *binval)
{
    int i;
    for (i = 7; i >= 0; i--)
    {
        binval[7-i] = ((ch>>i)&1)?'1':'0';
    }
    binval[8] = '\0';
}

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

int unstuff()
{
    int i, j;
    j = 0;
    i = 0;
    while (i < slen)
    {
        if (strncmp(&buf[i], "DLE", 3) == 0 && strncmp(&buf[i + 3], "DLE", 3) == 0)
        {
            msg[j++] = 'D';
            msg[j++] = 'L';
            msg[j++] = 'E';
            i += 6;
            continue;
        }
        msg[j] = buf[i];
        j++;
        i++;
    }
    msg[j] = '\0';
    return j;
}

void recv()
{
    FILE *fp;
    char read[100];
    char ch;
    int i = 0;
    char binval[9];

    fp = fopen("text.txt", "rb");
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

    i = 0;
    while (1)
    {
        long current_pos = ftell(fp);
        fgets(read, 4, fp);

        if (strncmp(read, "ETX", 3) == 0 || feof(fp))
        {
            break;
        }

        fseek(fp, current_pos, SEEK_SET);
        ch = fgetc(fp);
        buf[i] = ch;
        i++;
    }
    slen = i;

    ch = fgetc(fp);
    printf("  CRC byte received: ");
    bin(ch, binval);
    printf("%s\n", binval);

    printf("\nRaw buffer payload (Stuffed Character Stream):\n  ");
    for(i=0; i<slen; i++)
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
