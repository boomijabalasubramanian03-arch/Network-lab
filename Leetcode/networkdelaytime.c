#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INF 1000000000

int networkDelayTime(int** times, int timesSize, int* timesColSize, int n, int k) {
    int adj[105][105];
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            adj[i][j] = (i == j) ? 0 : INF;
        }
    }

    for (int i = 0; i < timesSize; i++) {
        int u = times[i][0];
        int v = times[i][1];
        int w = times[i][2];
        adj[u][v] = w;
    }

    int dist[105];
    bool visited[105];
    for (int i = 1; i <= n; i++) {
        dist[i] = INF;
        visited[i] = false;
    }
    dist[k] = 0;

    for (int count = 0; count < n; count++) {
        int u = -1;
        int min_d = INF;
        for (int i = 1; i <= n; i++) {
            if (!visited[i] && dist[i] < min_d) {
                min_d = dist[i];
                u = i;
            }
        }

        if (u == -1) break;
        visited[u] = true;

        for (int v = 1; v <= n; v++) {
            if (!visited[v] && adj[u][v] != INF) {
                if (dist[u] + adj[u][v] < dist[v]) {
                    dist[v] = dist[u] + adj[u][v];
                }
            }
        }
    }

    int max_time = 0;
    for (int i = 1; i <= n; i++) {
        if (dist[i] == INF) return -1;
        if (dist[i] > max_time) max_time = dist[i];
    }

    return max_time;
}
