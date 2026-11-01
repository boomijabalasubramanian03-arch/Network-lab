#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_STOPS 1000000

typedef struct Node {
    int bus;
    struct Node* next;
} Node;

#define HASH_SIZE 100003
Node* stopMap[HASH_SIZE];

void addStopBus(int stop, int bus) {
    int h = stop % HASH_SIZE;
    Node* n = (Node*)malloc(sizeof(Node));
    n->bus = bus;
    n->next = stopMap[h];
    stopMap[h] = n;
}

int numBusesToDestination(int** routes, int routesSize, int* routesColSize, int source, int target) {
    if (source == target) return 0;

    for (int i = 0; i < HASH_SIZE; i++) stopMap[i] = NULL;

    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            addStopBus(routes[i][j], i);
        }
    }

    int* q = (int*)malloc(sizeof(int) * routesSize);
    bool* visited_bus = (bool*)calloc(routesSize, sizeof(bool));
    int front = 0, rear = 0;

    int h = source % HASH_SIZE;
    Node* curr = stopMap[h];
    while (curr) {
        if (!visited_bus[curr->bus]) {
            for (int j = 0; j < routesColSize[curr->bus]; j++) {
                if (routes[curr->bus][j] == source) {
                    visited_bus[curr->bus] = true;
                    q[rear++] = curr->bus;
                    break;
                }
            }
        }
        curr = curr->next;
    }

    int bus_count = 0;
    while (front < rear) {
        bus_count++;
        int level_size = rear - front;
        for (int k = 0; k < level_size; k++) {
            int bus = q[front++];

            for (int j = 0; j < routesColSize[bus]; j++) {
                int stop = routes[bus][j];
                if (stop == target) {
                    free(q);
                    free(visited_bus);
                    return bus_count;
                }

                int sh = stop % HASH_SIZE;
                Node* nextBusNode = stopMap[sh];
                while (nextBusNode) {
                    int nxt = nextBusNode->bus;
                    if (!visited_bus[nxt]) {
                        visited_bus[nxt] = true;
                        q[rear++] = nxt;
                    }
                    nextBusNode = nextBusNode->next;
                }
            }
        }
    }

    free(q);
    free(visited_bus);
    return -1;
}
