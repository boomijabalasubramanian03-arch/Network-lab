#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define SERVER_PORT 5000
int s;
int len;
char buffer[256];
struct sockaddr_in servAddr;
struct sockaddr_in clntAddr;
socklen_t clntAddrLen;
int main(void)
{
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
    printf("Server started. Waiting for messages on port %d...\n", SERVER_PORT);
        clntAddrLen = sizeof(clntAddr);
        len = recvfrom(s, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr*)&clntAddr, &clntAddrLen);
        if (len < 0)
        {
            perror("Error: recvfrom failed!");
        }
        buffer[len] = '\0';
        printf("Received from client: %s\n", buffer);
    close(s);
    return 0;
}
