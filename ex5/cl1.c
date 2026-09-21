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
struct sockaddr_in servAddr;
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Error: two arguments are needed\n");
        exit(1);
    }
    servName = argv[1];
    servPort = atoi(argv[2]);
    printf("Enter string to send: ");
    fgets(string, sizeof(string), stdin);
    string[strcspn(string, "\n")] = '\0';
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
    len = sendto(s, string, strlen(string), 0,
                  (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (len < 0)
    {
        perror("Error: sendto failed!");
        exit(1);
    }
    printf("Sent to server: %s\n", string);
    close(s);
    return 0;
}
