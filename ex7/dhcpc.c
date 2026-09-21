#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSZ 1024

int sfd, n;
struct sockaddr_in saddr;
char buf[BUFSZ];
char cn[100];
int ch;
socklen_t len;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &saddr.sin_addr);
    len = sizeof(saddr);

    // 1. Get client identity
    printf("Enter client name: ");
    scanf("%99s", cn); // FIXED: Added bounds check to prevent buffer overflow

    // 2. Send DHCP Discover
    printf("Sending DHCP Discover to server...\n");
    sendto(sfd, cn, strlen(cn), 0, (struct sockaddr *)&saddr, len);

    // 3. Receive DHCP Offer Menu
    memset(buf, 0, BUFSZ);
    n = recvfrom(sfd, buf, BUFSZ - 1, 0, (struct sockaddr *)&saddr, &len);
    if (n < 0) { perror("recvfrom failed"); close(sfd); exit(1); }
    buf[n] = '\0';
    printf("\nReceived DHCP Offer from server:\n%s\n", buf);

    // 4. Input the subnet choice index
    printf("\nEnter subnet choice index: ");
    if (scanf("%d", &ch) != 1) {
        printf("Invalid input type.\n");
        close(sfd);
        return 1;
    }

    // 5. Send Formatted DHCP Request String
    printf("Sending DHCP Request for subnet %d...\n", ch);
    memset(buf, 0, BUFSZ);
    snprintf(buf, sizeof(buf), "Request %s %d", cn, ch); // FIXED: Safer string printing
    sendto(sfd, buf, strlen(buf), 0, (struct sockaddr *)&saddr, len);

    // 6. Receive final DHCP Acknowledgment
    memset(buf, 0, BUFSZ);
    n = recvfrom(sfd, buf, BUFSZ - 1, 0, (struct sockaddr *)&saddr, &len);
    if (n < 0) { perror("recvfrom failed"); close(sfd); exit(1); }
    buf[n] = '\0';

    printf("\nDHCP Information acknowledged:\n%s\n", buf);

    close(sfd);
    return 0;
}
