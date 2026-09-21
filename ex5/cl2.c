#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int s;
int len;
char* servName;
int servPort;
char string[256];
char buffer[256 + 1];
struct sockaddr_in servAddr;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Error: two arguments are needed! Usage: %s <server-ip> <port>\n", argv[0]);
        exit(1);
    }

    servName = argv[1];
    servPort = atoi(argv[2]);

    // Get string from user
    printf("Enter string to send: ");
    fgets(string, sizeof(string), stdin);
    string[strcspn(string, "\n")] = '\0';   // remove trailing newline

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

    // Send string
    len = sendto(s, string, strlen(string), 0,
                  (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (len < 0)
    {
        perror("Error: sendto failed!");
        exit(1);
    }

    // Receive reversed string
    len = recvfrom(s, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (len < 0)
    {
        perror("Error: recvfrom failed!");
        exit(1);
    }

    buffer[len] = '\0';
    printf("Reversed string received: %s\n", buffer);

    close(s);
    return 0;
}
