#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>

#define PORT 8080
#define SIZE 1024

int main()
{
    int sfd;
    struct sockaddr_in saddr, caddr, last_client;
    socklen_t clen;
    char buf[SIZE];
    int has_last_client = 0;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) { perror("socket"); exit(1); }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("bind");
        close(sfd);
        exit(1);
    }

    printf("--- Chat Server Started ---\n");
    printf("Waiting for a client to message you...\n\n");

    fd_set readfds;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int activity = select(sfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) { perror("select"); continue; }

        if (FD_ISSET(sfd, &readfds))
        {
            clen = sizeof(caddr);
            int n = recvfrom(sfd, buf, SIZE - 1, 0, (struct sockaddr *)&caddr, &clen);
            if (n > 0) {
                buf[n] = '\0';
                buf[strcspn(buf, "\r\n")] = 0;

                last_client = caddr;
                has_last_client = 1;

                if (strncmp(buf, "bye", 3) == 0) {
                    printf("\n[Client %d left the chat]\n",
                           ntohs(caddr.sin_port));
                    has_last_client = 0;
                    continue;
                }

                printf("\nClient: %s\n", buf);
                printf("Server: ");
                fflush(stdout);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                buf[strcspn(buf, "\r\n")] = 0;

                if (has_last_client) {
                    sendto(sfd, buf, strlen(buf), 0,
                           (struct sockaddr *)&last_client, sizeof(last_client));

                } else {
                    printf("\n[System] No client has sent a message yet. Please wait for a client request.\n");
                }
            }
        }
    }

    close(sfd);
    return 0;
}
