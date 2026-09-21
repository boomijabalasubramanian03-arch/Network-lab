#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8003

char generator[20];
int  genLen;
char work[200];
char rem[10];

int s;
int len;
struct sockaddr_in servAddr;
struct sockaddr_in clntAddr;
socklen_t clntAddrLen;
char buffer[256];
char* sep;

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

int main(void)
{
    int i, allZero;

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        perror("Error: socket failed!");
        exit(1);
    }

    if (bind(s, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error: bind failed!");
        exit(1);
    }

    printf("CRC server started on port %d...\n", SERVER_PORT);


        clntAddrLen = sizeof(clntAddr);

        len = recvfrom(s, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr*)&clntAddr, &clntAddrLen);
        if (len < 0)
        {
            perror("Error: recvfrom failed!");

        }

        buffer[len] = '\0';

        sep = strchr(buffer, '|');
        if (sep == NULL)
        {
            printf("Malformed message received, ignoring.\n");

        }

        *sep = '\0';
        strcpy(generator, buffer);
        genLen = strlen(generator);
        strcpy(work, sep + 1);

        printf("Generator  : %s\n", generator);
        printf("Codeword   : %s\n", work);

        modulo2Divide();

        allZero = 1;
        for (i = 0; i < genLen - 1; i++)
        {
            if (rem[i] != '0')
            {
                allZero = 0;
                break;
            }
        }

        if (allZero)
            strcpy(buffer, "No error detected - packet accepted");
        else
            strcpy(buffer, "Error detected - packet discarded");

        sendto(s, buffer, strlen(buffer), 0,
               (struct sockaddr*)&clntAddr, clntAddrLen);


    close(s);
    return 0;
}
