#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

char generator[32];
int  genLen;
char data[100];
char binaryData[800];
char work[1000];
char rem[32];
char codeword[1000];
char message[1100];
char reply[256];
char fileName[100];

int s;
int len;
char* servName;
int servPort;
struct sockaddr_in servAddr;
FILE* fp;

void parsePolynomial(const char* polyInput, char* outputBinary)
{
    int coeffs[32] = {0};
    int maxDegree = 0;
    char copy[100];
    strncpy(copy, polyInput, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char* token = strtok(copy, "+");
    while (token != NULL)
    {
        int deg = 0;
        char* xPtr = strchr(token, 'x');
        if (!xPtr) xPtr = strchr(token, 'X');

        if (xPtr)
        {
            char* caretPtr = strchr(xPtr, '^');
            if (caretPtr)
                deg = atoi(caretPtr + 1);
            else
                deg = 1;
        }
        else if (strchr(token, '1'))
        {
            deg = 0;
        }

        if (deg >= 0 && deg < 32)
        {
            coeffs[deg] = 1;
            if (deg > maxDegree) maxDegree = deg;
        }
        token = strtok(NULL, "+");
    }

    int idx = 0;
    int i;
    for (i = maxDegree; i >= 0; i--)
    {
        outputBinary[idx++] = coeffs[i] ? '1' : '0';
    }
    outputBinary[idx] = '\0';
}

void textToBinaryString(const char* text, char* output)
{
    output[0] = '\0';
    int i;
    int bit;
    for (i = 0; text[i] != '\0'; i++)
    {
        char ch = text[i];
        int i;
        for (bit = 7; bit >= 0; bit--)
        {
            if ((ch >> bit) & 1)
                strcat(output, "1");
            else
                strcat(output, "0");
        }
    }
}

void modulo2Divide(void)
{
    int i, j, n;
    n = strlen(work);
    for (i = 0; i <= n - genLen; i++)
    {
        if (work[i] == '1')
        {
            for (j = 0; j < genLen; j++)
                work[i + j] = (work[i + j] == generator[j]) ? '0' : '1';
        }
    }
    strcpy(rem, work + (n - (genLen - 1)));
}

int main(int argc, char* argv[])
{
    int i, pos;
    char choice;
    char rawPolyInput[100];

    if (argc != 3)
    {
        printf("Error: two arguments are needed\n", argv[0]);
        exit(1);
    }

    servName = argv[1];
    servPort = atoi(argv[2]);

    printf("Enter generator polynomial: ");
    fgets(rawPolyInput, sizeof(rawPolyInput), stdin);
    rawPolyInput[strcspn(rawPolyInput, "\n")] = '\0';

    parsePolynomial(rawPolyInput, generator);
    genLen = strlen(generator);
    printf(" Polynomial Binary: %s (Length: %d)\n\n", generator, genLen);

    printf("Enter input file name: ");
    fgets(fileName, sizeof(fileName), stdin);
    fileName[strcspn(fileName, "\n")] = '\0';

    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error: could not open file!");
        exit(1);
    }
    fgets(data, sizeof(data), fp);
    data[strcspn(data, "\n")] = '\0';
    fclose(fp);

    printf("Data : %s\n", data);
    textToBinaryString(data, binaryData);
    printf(" Binary form   : %s\n\n", binaryData);

    strcpy(work, binaryData);
    for (i = 0; i < genLen - 1; i++)
        strcat(work, "0");
    modulo2Divide();

    strcpy(codeword, binaryData);
    strcat(codeword, rem);

    printf("Generated CRC bits : %s\n", rem);
    printf("Codeword to send   : %s\n\n", codeword);

    printf("Do you want to change a bit  (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y')
    {
        printf("Enter bit position to flip (1 to %d): ", (int)strlen(codeword));
        scanf("%d", &pos);
        pos = pos - 1;
        if (pos >= 0 && pos < (int)strlen(codeword))
            codeword[pos] = (codeword[pos] == '0') ? '1' : '0';
        printf("Modified codeword  : %s\n", codeword);
    }

    strcpy(message, generator);
    strcat(message, "|");
    strcat(message, codeword);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, servName, &servAddr.sin_addr);
    servAddr.sin_port = htons(servPort);

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        perror("Error: socket failed!");
        exit(1);
    }

    len = sendto(s, message, strlen(message), 0,
                  (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (len < 0)
    {
        perror("Error: sendto failed!");
        exit(1);
    }

    len = recvfrom(s, reply, sizeof(reply) - 1, 0, NULL, NULL);
    if (len < 0)
    {
        perror("Error: recvfrom failed!");
        exit(1);
    }

    reply[len] = '\0';
    printf("Server response: %s\n", reply);

    close(s);
    return 0;
}
