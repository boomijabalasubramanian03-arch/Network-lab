#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIZE 1024

int sfd, n;
struct sockaddr_in saddr;
socklen_t len;
char buf[SIZE];

int main(int argc, char *argv[])
{
    if (argc != 3) { printf("Usage: %s <server_ip> <port>\n", argv[0]); exit(1); }

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) { perror("socket"); exit(1); }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &saddr.sin_addr);
    len = sizeof(saddr);

    printf("Type 'bye' to end the chat.\n\n");

    while (1)
    {
        printf("Client: ");
        fgets(buf, SIZE, stdin);
        buf[strcspn(buf, "\n")] = '\0';

        sendto(sfd, buf, strlen(buf), 0, (struct sockaddr *)&saddr, len);

        if (strncmp(buf, "bye", 3) == 0)
            break;

        n = recvfrom(sfd, buf, SIZE - 1, 0, NULL, NULL);
        if (n < 0) { perror("recvfrom"); break; }
        buf[n] = '\0';
        printf("Server: %s\n", buf);
    }

    close(sfd);
    return 0;
}
