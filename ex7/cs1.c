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
    int has_last_client = 0; // Tracks if any client has messaged us yet

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
        FD_SET(sfd, &readfds);          // Listen for incoming network text
        FD_SET(STDIN_FILENO, &readfds);     // Listen for server keyboard typing

        int activity = select(sfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) { perror("select"); continue; }

        // 1. Server received a message from a client
        if (FD_ISSET(sfd, &readfds))
        {
            clen = sizeof(caddr);
            int n = recvfrom(sfd, buf, SIZE - 1, 0, (struct sockaddr *)&caddr, &clen);
            if (n > 0) {
                buf[n] = '\0';
                buf[strcspn(buf, "\r\n")] = 0; // Clean newline layout

                // Save this client's details so the server knows who to reply to
                last_client = caddr;
                has_last_client = 1;

                if (strncmp(buf, "bye", 3) == 0) {
                    printf("\n[Client %d left the chat]\n",
                           ntohs(caddr.sin_port));
                    has_last_client = 0; // Clear the reply target
                    continue;
                }

                // Normal message display layout
                printf("\nClient: %s\n", buf);
                printf("Server: "); // CHANGED: Replaced "You:" with "Server:"
                fflush(stdout);
            }
        }

        // 2. Server operator typed a reply and pressed Enter
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                buf[strcspn(buf, "\r\n")] = 0;

                // Send message to the client who spoke last
                if (has_last_client) {
                    sendto(sfd, buf, strlen(buf), 0,
                           (struct sockaddr *)&last_client, sizeof(last_client));

                    // The terminal naturally displays what you type next to "Server: ".
                    // No duplicate print statements here, keeping the layout clean.
                } else {
                    printf("\n[System] No client has sent a message yet. Please wait for a client request.\n");
                }
            }
        }
    }

    close(sfd);
    return 0;
}
