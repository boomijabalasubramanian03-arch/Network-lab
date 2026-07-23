#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define N 2
#define B 8000

struct Node {
    char u[50];
    char ip[20];
    char mac[20];
    int p;
};

struct Header {
    int sp;
    int dp;
    char sip[20];
    char dip[20];
    char smac[20];
    char dmac[20];
};

struct Header hd;
char msg[100];
char stf[200];
int len;
int slen;

struct Node tbl[N] = {
    {"www.google.com", "192.168.166.242", "43:50:94:25:49:12", 49101},
    {"www.email.com", "192.168.246.228", "07:C1:F0:7C:1F:07", 9208}
};

void bin(char ch, char *bv) {
    int i = 7;
    while (i >= 0) {
        bv[7 - i] = ((ch >> i) & 1) ? '1' : '0';
        i--;
    }
    bv[8] = '\0';
}

int map(char *su, char *du) {
    int si = -1, di = -1, i = 0;
    while (i < N) {
        if (strcmp(tbl[i].u, su) == 0) si = i;
        if (strcmp(tbl[i].u, du) == 0) di = i;
        i++;
    }
    if (si == -1 || di == -1) return 0;

    hd.sp = tbl[si].p;
    hd.dp = tbl[di].p;
    strcpy(hd.sip, tbl[si].ip);
    strcpy(hd.dip, tbl[di].ip);
    strcpy(hd.smac, tbl[si].mac);
    strcpy(hd.dmac, tbl[di].mac);
    return 1;
}

int stuff(char *src, char *dst) {
    int i = 0, j = 0;
    int sl = strlen(src);
    while (i < sl) {
        if (strncmp(&src[i], "ETX", 3) == 0) {
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

void stream(char *src, char *bs) {
    char tb[9];
    int i = 0;
    bs[0] = '\0';
    while (src[i] != '\0') {
        bin(src[i], tb);
        strcat(bs, tb);
        i++;
    }
}

void send() {
    FILE *fp;
    int i = 0;
    fp = fopen("text.txt", "w");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }
    fputs("SYN", fp);
    fputs("SYN", fp);
    fputs("SOH", fp);
    fwrite(&hd, sizeof(struct Header), 1, fp);
    fputs("STX", fp);
    while (i < slen) {
        fputc(stf[i], fp);
        i++;
    }
    fputs("ETX", fp);
    fputc('0', fp);
    fclose(fp);
    printf("\nFrame successfully written to text.txt in BISYNC format!\n");
}

int main() {
    char su[50], du[50], ch;
    char bv[9];
    int ps, fs, i, k, p, b;
    FILE *fp;

    printf("Enter Source URL : ");
    scanf("%49s", su);
    printf("Enter Destination URL : ");
    scanf("%49s", du);

    if (!map(su, du)) {
        printf("Error: Configuration parameters could not be mapped from table.\n");
        return 1;
    }

    printf("\nSTORED VALUES IN STRUCTURE TABLE\nMapped successfully.\n");
    printf("Enter Payload (no spaces): ");
    scanf("%99s", msg);
    len = strlen(msg);

    printf("Enter Packet Size (in bits): ");
    scanf("%d", &ps);
    printf("Enter Frame Size (in bits): ");
    scanf("%d", &fs);

    char rpb[B];
    stream(msg, rpb);

    printf("\n====================================\nAPPLICATION LAYER\n====================================\n");
    printf("Application Data : %s\n", msg);
    printf("Output stream (Data bits): ");
    i = 0;
    while (msg[i] != '\0') {
        bin(msg[i], bv);
        printf("%s ", bv);
        i++;
    }
    printf("\n");

    printf("\n====================================\nTRANSPORT LAYER\n====================================\n");
    printf("Added TCP/UDP Segment Headers:\n");
    printf("  Source Port bits     : ");
    i = 15; while (i >= 0) { printf("%d", (hd.sp >> i) & 1); i--; }
    printf("\n  Destination Port bits: ");
    i = 15; while (i >= 0) { printf("%d", (hd.dp >> i) & 1); i--; }
    printf("\n\n[Appended Output stream] -> Segment (Ports + Data):\n");
    i = 15; while (i >= 0) { printf("%d", (hd.sp >> i) & 1); i--; }
    printf(" ");
    i = 15; while (i >= 0) { printf("%d", (hd.dp >> i) & 1); i--; }
    printf(" ");
    i = 0; while (msg[i] != '\0') { bin(msg[i], bv); printf("%s ", bv); i++; }
    printf("\n");

    char phb[40] = "";
    i = 15; while (i >= 0) { sprintf(&phb[strlen(phb)], "%d", (hd.sp >> i) & 1); i--; }
    i = 15; while (i >= 0) { sprintf(&phb[strlen(phb)], "%d", (hd.dp >> i) & 1); i--; }

    char ihb[400] = "";
    k = 0; while (k < (int)strlen(hd.sip)) { bin(hd.sip[k], bv); strcat(ihb, bv); k++; }
    k = 0; while (k < (int)strlen(hd.dip)) { bin(hd.dip[k], bv); strcat(ihb, bv); k++; }

    char mhb[400] = "";
    k = 0; while (k < (int)strlen(hd.smac)) { bin(hd.smac[k], bv); strcat(mhb, bv); k++; }
    k = 0; while (k < (int)strlen(hd.dmac)) { bin(hd.dmac[k], bv); strcat(mhb, bv); k++; }

    char cnb[B];
    strcpy(cnb, phb);
    strcat(cnb, rpb);
    int npl = strlen(cnb);
    int tp = (npl + ps - 1) / ps;

    printf("\n====================================\nNETWORK LAYER\n====================================\n");
    printf("Added IP Packet Headers:\n");
    printf("  Source IP bits     : ");
    k = 0; while (k < (int)strlen(hd.sip)) { bin(hd.sip[k], bv); printf("%s ", bv); k++; }
    printf("\n  Destination IP bits: ");
    k = 0; while (k < (int)strlen(hd.dip)) { bin(hd.dip[k], bv); printf("%s ", bv); k++; }
    printf("\n");

    char mpb[B] = "";
    p = 0;
    while (p < tp) {
        int sb = p * ps;
        char cpp[B] = "";

        b = 0;
        while (b < ps) {
            int ci = sb + b;
            if (ci < npl) {
                char tmp[2] = {cnb[ci], '\0'};
                strcat(cpp, tmp);
            } else {
                strcat(cpp, "0");
            }
            b++;
        }

        printf("\nPACKET %d (Bits %d to %d)\n", p + 1, sb, sb + ps - 1);
        printf("[Appended Output stream] -> Packet (IPs + Ports + Data Sub-slice):\n");

        k = 0;
        while (ihb[k] != '\0') {
            printf("%c", ihb[k]);
            if ((k + 1) % 8 == 0) printf(" ");
            k++;
        }
        printf(" ");

        k = 0;
        while (cpp[k] != '\0') {
            printf("%c", cpp[k]);
            if ((k + 1) % 8 == 0) printf(" ");
            k++;
        }
        printf("\n");

        strcat(mpb, cpp);
        p++;
    }

    printf("\n====================================\nDATA LINK LAYER (BIT-BASED SPLITTING WITH PADDING & STUFFING)\n====================================\n");
    slen = stuff(msg, stf);
    printf("[Character Transparency Processing via Byte-Stuffing Tracing]:\n");
    printf("  Original Message String : %s\n", msg);
    printf("  Stuffed Output Stream   : %s\n\n", stf);

    int fpp = ps / fs;
    int cg = 0;

    p = 0;
    while (p < tp) {
        printf("------------------------------------\n");
        printf("PACKET %d FRAME SEGMENTATION DETAILS\n", p + 1);
        printf("------------------------------------\n");
        printf("Added Frame MAC Headers:\n");
        printf("  Source MAC bits     : ");
        k = 0; while (k < (int)strlen(hd.smac)) { bin(hd.smac[k], bv); printf("%s ", bv); k++; }
        printf("\n  Destination MAC bits: ");
        k = 0; while (k < (int)strlen(hd.dmac)) { bin(hd.dmac[k], bv); printf("%s ", bv); k++; }
        printf("\n\n[Appended Output stream Processed inside Frame Windows]:\n");

        int f = 0;
        while (f < fpp) {
            int fbs = (p * ps) + (f * fs);
            char cfp[B] = "";

            b = 0;
            while (b < fs) {
                int gi = fbs + b;
                if (gi < (int)strlen(mpb)) {
                    char tmp[2] = {mpb[gi], '\0'};
                    strcat(cfp, tmp);
                } else {
                    strcat(cfp, "0");
                }
                b++;
            }

            printf("  Frame %d (Bits %d to %d) -> Full Frame bits:\n  ", f + 1, f * fs, (f * fs) + fs - 1);

            k = 0;
            while (mhb[k] != '\0') {
                printf("%c", mhb[k]);
                if ((k + 1) % 8 == 0) printf(" ");
                k++;
            }
            printf("  %s\n", cfp);
            cg++;
            f++;
        }
        printf("\n");
        p++;
    }

    printf("====================================\n");
    printf("TRANSMISSION METRICS SUMMARY\n");
    printf("====================================\n");
    printf("Total Packets Generated : %d\n", tp);
    printf("Total Frames Generated  : %d\n", cg);

    send();

    printf("\n====================================\n");
    printf("PHYSICAL LAYER\n");
    printf("====================================\n");
    printf("[Appended Output stream] -> Final Bitstream over physical medium:\n");

    fp = fopen("text.txt", "r");
    if (fp == NULL) {
        printf("Error opening file for reading\n");
        return 1;
    }

    ch = fgetc(fp);
    while (ch != EOF) {
        bin(ch, bv);
        printf("%s ", bv);
        ch = fgetc(fp);
    }
    printf("\n");
    fclose(fp);

    return 0;
}
