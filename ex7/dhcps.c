#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PORT 2020
#define MAXSUB 10
#define BUFSZ 1024

struct sub
{
    int req;
    int alc;
    int pre;
    unsigned int net;
    unsigned int bc;
    unsigned int msk;
};

int sfd, n, ns, bp, i;
char buf[BUFSZ];
char bip[50];
struct sockaddr_in saddr, caddr;
socklen_t len;
struct sub sb[MAXSUB];
unsigned int bnet, cur;

unsigned int ip2int(char *ip)
{
    int a, b, c, d;
    sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);
    unsigned int r = 0;
    r = r + (unsigned int)a * 256 * 256 * 256;
    r = r + (unsigned int)b * 256 * 256;
    r = r + (unsigned int)c * 256;
    r = r + (unsigned int)d;
    return r;
}

void int2ip(unsigned int ip, char *s)
{
    unsigned int a = ip / (256 * 256 * 256);
    unsigned int b = (ip / (256 * 256)) % 256;
    unsigned int c = (ip / 256) % 256;
    unsigned int d = ip % 256;
    sprintf(s, "%u.%u.%u.%u", a, b, c, d);
}

int npow2(int n)
{
    int p = 1;
    while (p < n)
        p = p * 2;
    return p;
}

int calpre(int alc)
{
    int p = 32;
    int t = alc;
    while (t > 1)
    {
        t = t / 2;
        p--;
    }
    return p;
}

unsigned int mkmsk(int p)
{
    if (p == 0)
        return 0;
    return (unsigned int)0xFFFFFFFF << (32 - p);
}

int cmp(const void *a, const void *b)
{
    return ((struct sub *)b)->req - ((struct sub *)a)->req;
}

int main()
{
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) { perror("Socket creation failed"); exit(1); }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
    {
        perror("Bind failed");
        close(sfd);
        exit(1);
    }

    printf("DHCP server configuration\n");
    printf("Enter base network address: ");
    scanf("%s", bip);
    printf("Enter base prefix: ");
    scanf("%d", &bp);
    printf("Enter no. of subnets: ");
    scanf("%d", &ns);

    for (i = 0; i < ns; i++)
    {
        printf("Enter no. of hosts required for subnet %d: ", i + 1);
        scanf("%d", &sb[i].req);
    }

    qsort(sb, ns, sizeof(struct sub), cmp);

    bnet = ip2int(bip);
    cur = bnet;

    for (i = 0; i < ns; i++)
    {
        sb[i].alc = npow2(sb[i].req + 2);
        sb[i].pre = calpre(sb[i].alc);
        sb[i].net = cur;
        sb[i].msk = mkmsk(sb[i].pre);
        sb[i].bc = sb[i].net + sb[i].alc - 1;

        char nt[50], mk[50], bc[50];
        int2ip(sb[i].net, nt);
        int2ip(sb[i].msk, mk);
        int2ip(sb[i].bc, bc);

        printf("Subnet %d: required=%d, allocated=%d, prefix=%d, network=%s, mask=%s, broadcast=%s\n",
               i, sb[i].req, sb[i].alc, sb[i].pre, nt, mk, bc);

        cur = cur + sb[i].alc;
    }

    printf("\nDHCP server running on port %d...\n", PORT);

    while (1)
    {
        memset(buf, 0, BUFSZ);
        len = sizeof(caddr);
        n = recvfrom(sfd, buf, BUFSZ - 1, 0, (struct sockaddr *)&caddr, &len);
        if (n < 0) { perror("recvfrom"); continue; }
        buf[n] = '\0';

        // FIXED: The single main process now handles requests directly instead of using fork()
        if (strncmp(buf, "Request", 7) == 0)
        {
            char cn[100];
            int ch = -1;
            sscanf(buf, "Request %s %d", cn, &ch);

            printf("[Server] Processing Request from client: %s for Subnet: %d\n", cn, ch);

            if (ch < 0 || ch >= ns)
            {
                strcpy(buf, "Invalid subnet choice");
                sendto(sfd, buf, strlen(buf), 0, (struct sockaddr *)&caddr, len);
                continue;
            }

            char nt2[50], mk2[50], bc2[50], us[50], ue[50];
            int2ip(sb[ch].net, nt2);
            int2ip(sb[ch].msk, mk2);
            int2ip(sb[ch].bc, bc2);
            int2ip(sb[ch].net + 1, us);
            int2ip(sb[ch].bc - 1, ue);

            memset(buf, 0, BUFSZ);
            sprintf(buf, "DHCP ACK\nClient Name: %s\nPrefix: %d\nNetwork: %s\nMask: %s\nBroadcast: %s\nUsable Range: %s - %s\n",
                    cn, sb[ch].pre, nt2, mk2, bc2, us, ue);

            sendto(sfd, buf, strlen(buf), 0, (struct sockaddr *)&caddr, len);
            printf("[Server] Sent DHCP ACK configuration to %s\n\n", cn);
        }
        else
        {
            // Handles the initial discover step cleanly
            printf("[Server] Received DHCP Discover from client: %s\n", buf);

            char response[BUFSZ];
            sprintf(response, "DHCP OFFER");
            for (i = 0; i < ns; i++)
            {
                char ln[100];
                sprintf(ln, "\n%d) prefix=%d hosts available=%d", i, sb[i].pre, sb[i].alc - 2);
                strcat(response, ln);
            }
            sendto(sfd, response, strlen(response), 0, (struct sockaddr *)&caddr, len);
            printf("[Server] Dispatched network options map to client.\n");
        }
    }

    close(sfd);
    return 0;
}
