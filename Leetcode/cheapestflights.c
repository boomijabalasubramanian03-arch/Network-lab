#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 1000000000

int findCheapestPrice(int n, int** flights, int flightsSize, int* flightsColSize, int src, int dst, int k) {
    int dist[105];
    int temp[105];
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
    }
    dist[src] = 0;

    for (int step = 0; step <= k; step++) {
        for (int i = 0; i < n; i++) {
            temp[i] = dist[i];
        }

        for (int e = 0; e < flightsSize; e++) {
            int u = flights[e][0];
            int v = flights[e][1];
            int price = flights[e][2];

            if (dist[u] != INF && dist[u] + price < temp[v]) {
                temp[v] = dist[u] + price;
            }
        }

        for (int i = 0; i < n; i++) {
            dist[i] = temp[i];
        }
    }

    return (dist[dst] == INF) ? -1 : dist[dst];
}
