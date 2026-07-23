#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

#define MAX_DATA 1000
#define MAX_NODES 50

struct Node
{
    char url[50];
    int ip[4];
    char mac[18];
    int port;
};

struct Node table[MAX_NODES];
int entry = 0;

int activeS = -1;
int activeD = -1;

char data[MAX_DATA];
char filename[50];
int pSize;
int fSize;



void bin8(int num)
{
    int i;
    for(i=7;i>=0;i--)
        printf("%d",(num>>i)&1);
}

void bin16(int num)
{
    int i;
    for(i=15;i>=0;i--)
        printf("%d",(num>>i)&1);
}

void stringBin(char str[])
{
    int i;
    for(i=0;str[i]!='\0';i++)
    {
        bin8((unsigned char)str[i]);
        printf(" ");
    }
}

void ipBin(int ip[])
{
    int i;
    for(i=0;i<4;i++)
    {
        bin8(ip[i]);
        printf(" ");
    }
}

void macBin(char mac[])
{
    int value;
    char temp[3];
    int i,j=0;

    for(i=0;mac[i]!='\0';i++)
    {
        if(mac[i]==':')
            continue;

        temp[j++]=mac[i];

        if(j==2)
        {
            temp[2]='\0';
            value=(int)strtol(temp,NULL,16);
            bin8(value);
            printf(" ");
            j=0;
        }
    }
}



void generateIP(char url[],int ip[])
{
    int hash=0;
    int i;

    for(i=0;url[i]!='\0';i++)
        hash += url[i]*(i+1);

    ip[0]=192;
    ip[1]=168;
    ip[2]=(hash%254)+1;
    ip[3]=((hash*3)%254)+1;
}

void generateMAC(char url[],char mac[])
{
    int hash=0;
    int i;

    for(i=0;url[i]!='\0';i++)
        hash += url[i]*(i+7);

    sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",
            hash&0xFF,
            (hash>>2)&0xFF,
            (hash>>4)&0xFF,
            (hash>>6)&0xFF,
            (hash>>8)&0xFF,
            (hash>>10)&0xFF);
}

int generatePort()
{
    return rand()%50000+1024;
}



void addNetworkPair()
{
    if(entry + 2 > MAX_NODES)
    {
        printf("Error: Table array registry structure full.\n");
        return;
    }


    printf("Enter Source URL : ");
    scanf("%49s", table[entry].url);
    generateIP(table[entry].url, table[entry].ip);
    generateMAC(table[entry].url, table[entry].mac);
    table[entry].port = generatePort();
    entry++;

    printf("Enter Destination URL : ");
    scanf("%49s", table[entry].url);
    generateIP(table[entry].url, table[entry].ip);
    generateMAC(table[entry].url, table[entry].mac);
    table[entry].port = generatePort();
    entry++;
}

void displayNodes()
{
    int i;
    printf("\nSTORED VALUES IN STRUCTURE TABLE");

    if(entry == 0)
    {
        printf("\n[Table Empty. No nodes found.]\n");
        return;
    }

    for(i = 0; i < entry; i++)
    {
        printf("\nNODE INDEX [%d]", i);
        printf("\nURL  : %s", table[i].url);
        printf("\nIP   : %d.%d.%d.%d", table[i].ip[0], table[i].ip[1], table[i].ip[2], table[i].ip[3]);
        printf("\nMAC  : %s", table[i].mac);
        printf("\nPORT : %d\n", table[i].port);
    }
}

void selectSessionEndpoints()
{
    displayNodes();
    if(entry < 2)
    {
        printf("\nError: Please add at least one network pair first.\n");
        return;
    }
    printf("\nEnter Active Source Index: ");
    scanf("%d", &activeS);
    printf("Enter Active Destination Index: ");
    scanf("%d", &activeD);

    if(activeS < 0 || activeS >= entry || activeD < 0 || activeD >= entry)
    {
        printf("Invalid mapping indices choice. Session reset.\n");
        activeS = -1; activeD = -1;
    }
}

void getFileData()
{
    FILE *fp;

    printf("\nEnter filename : ");
    scanf("%49s",filename);

    fp=fopen(filename,"r");

    if(fp==NULL)
    {
        printf("\nFile not found.\n");
        exit(0);
    }

    fgets(data,MAX_DATA,fp);
    fclose(fp);

    data[strcspn(data,"\n")]='\0';
}

void getSizes()
{
    printf("\nEnter Packet Size (in bits): ");
    scanf("%d",&pSize);

    printf("Enter Frame Size (in bits): ");
    scanf("%d",&fSize);
}

void applicationLayer()
{
    int i;
    printf("\n====================================");
    printf("\nAPPLICATION LAYER\n");
    printf("====================================\n");
    printf("Application Data : %s\n",data);
    printf("Output stream (Data bits): ");
    for(i=0;data[i]!='\0';i++)
    {
        bin8((unsigned char)data[i]);
        printf(" ");
    }
    printf("\n");
}

void transportLayer()
{
    struct Node src = table[activeS];
    struct Node dst = table[activeD];

    printf("\n====================================");
    printf("\nTRANSPORT LAYER\n");
    printf("====================================\n");
    printf("Added TCP/UDP Segment Headers:\n");
    printf("  Source Port bits     : "); bin16(src.port); printf("\n");
    printf("  Destination Port bits: "); bin16(dst.port); printf("\n");

    printf("\n[Appended Output stream] -> Segment (Ports + Data):\n");
    bin16(src.port); printf(" ");
    bin16(dst.port); printf(" ");
    stringBin(data);
    printf("\n");
}

void networkLayer()
{
    struct Node src = table[activeS];
    struct Node dst = table[activeD];

    printf("\n====================================");
    printf("\nNETWORK LAYER\n");
    printf("====================================\n");
    printf("Added IP Packet Headers:\n");
    printf("  Source IP bits     : "); ipBin(src.ip); printf("\n");
    printf("  Destination IP bits: "); ipBin(dst.ip); printf("\n");

    printf("\n[Appended Output stream] -> Packet (IPs + Ports + Data):\n");
    ipBin(src.ip); printf(" ");
    ipBin(dst.ip); printf(" ");
    bin16(src.port); printf(" ");
    bin16(dst.port); printf(" ");
    stringBin(data);
    printf("\n");
}

void dataLinkLayer()
{
    struct Node src = table[activeS];
    struct Node dst = table[activeD];
    int dataLength;
    int packetCount = 0;
    int totalFrameCount = 0;
    int bitIndex = 0;
    int totalBits;
    int b;

    dataLength = strlen(data);
    totalBits = dataLength * 8;

    printf("\n====================================");
    printf("\nDATA LINK LAYER (BIT-BASED SPLITTING WITH PADDING)\n");
    printf("====================================\n");

    while (bitIndex < totalBits)
    {
        int packetStartBit = bitIndex;
        int packetEndBit = bitIndex + pSize - 1;
        int frameCount = 0;
        int currentFrameBit = packetStartBit;

        packetCount++;

        printf("\nPACKET %d (Bits %d to %d)\n", packetCount, packetStartBit, packetEndBit);
        printf("Added Frame Headers:\n");
        printf("  Source MAC bits     : "); macBin(src.mac); printf("\n");
        printf("  Destination MAC bits: "); macBin(dst.mac); printf("\n");

        printf("\n[Appended Output stream Processed inside Frame Windows]:\n");
        while (currentFrameBit <= packetEndBit)
        {
            int frameEndBit = currentFrameBit + fSize - 1;
            frameCount++;
            totalFrameCount++;

            printf("  Frame %d (Bits %d to %d) -> Full Frame bits: ", frameCount, currentFrameBit, frameEndBit);

            macBin(src.mac); printf(" ");
            macBin(dst.mac); printf(" ");

            for (b = currentFrameBit; b <= frameEndBit; b++)
            {
                if (b >= totalBits)
                {
                    printf("0");
                }
                else
                {
                    int charPos = b / 8;
                    int bitPos = 7 - (b % 8);
                    int bitValue = (data[charPos] >> bitPos) & 1;
                    printf("%d", bitValue);
                }
            }
            printf("\n");

            currentFrameBit += fSize;
        }

        bitIndex += pSize;
    }

    printf("\n====================================");
    printf("\nTRANSMISSION METRICS SUMMARY");
    printf("\n====================================");
    printf("\nTotal Packets Generated : %d", packetCount);
    printf("\nTotal Frames Generated  : %d\n", totalFrameCount);
}

void physicalLayer()
{
    struct Node src = table[activeS];
    struct Node dst = table[activeD];

    printf("\n====================================");
    printf("\nPHYSICAL LAYER\n");
    printf("====================================\n");
    printf("[Appended Output stream] -> Final Bitstream over physical medium:\n");

    macBin(src.mac); printf(" ");
    macBin(dst.mac); printf(" ");
    ipBin(src.ip); printf(" ");
    ipBin(dst.ip); printf(" ");
    bin16(src.port); printf(" ");
    bin16(dst.port); printf(" ");
    stringBin(data);
    printf("\n");
}

int deletenode(char *url) {
    int i, found = -1;
    for(i = 0; i < entry; i++) {
        if(strcmp(table[i].url, url) == 0) {
            found = i;
            break;
        }
    }
    if(found != -1) {
        for(i = found; i < entry - 1; i++) {
            table[i] = table[i + 1];
        }
        entry--;
        activeS = -1;
        activeD = -1;
        return 1;
    }
    return 0;
}



int main()
{
    srand((unsigned int)time(NULL));
    int choice;
    char input_url[50];
    while(1)
    {
       printf("\n=== NETWORK MENU ===\n");
       printf("1. Add network\n");
       printf("2. Delete\n");
       printf("3. View table\n");
       printf("4. Configure simulation session\n");
       printf("5. Generate frame\n");
       printf("6. Exit\n");
       printf("Enter your choice: ");
       if (scanf("%d", &choice) != 1)
       {
          printf("Invalid selection index input format.\n");
          break;
       }
       if (choice == 1)
       {
          addNetworkPair();
          printf("Network details configured successfully.\n");
       }
       else if (choice == 2)
       {
          printf("Enter URL to delete: ");
          scanf("%49s", input_url);
          while(getchar() != '\n');
             if(deletenode(input_url))
             {
                printf("Node deleted successfully.\n");
             }
             else
             {
                printf("URL not found in table.\n");
             }
       }
       else if (choice == 3)
       {
          displayNodes();
       }
       else if (choice == 4)
       {
          selectSessionEndpoints();
       }
       else if (choice == 5)
       {
          if (activeS == -1 || activeD == -1)
          {
             printf("Please map active tracking endpoints using Option 4 before generating simulations.\n");
             continue;
          }
          getFileData();
          getSizes();
          applicationLayer();
          transportLayer();
          networkLayer();
          dataLinkLayer();
          physicalLayer();
       }
       else if (choice == 6)
       {
          printf("Program stopped.\n");
          exit(0);
       }
       else
       {
          printf("Invalid selection. Try again.\n");
       }
    }
       return 0;
}
