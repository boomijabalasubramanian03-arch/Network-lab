#include <stdio.h>

#define INF 1000000000

int dist[405][405];

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (i == j) dist[i][j] = 0;
            else dist[i][j] = INF;
        }
    }

    for (int i = 0; i < m; i++) {
        int u, v, w;
        scanf("%d %d %d", &u, &v, &w);
        dist[u][v] = w;
    }

    for (int k = 1; k <= n; k++) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF) {
                    if (dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                    }
                }
            }
        }
    }

    int q;
    if (scanf("%d", &q) == 1) {
        while (q--) {
            int u, v;
            scanf("%d %d", &u, &v);
            if (dist[u][v] == INF) {
                printf("-1\n");
            } else {
                printf("%d\n", dist[u][v]);
            }
        }
    }

    return 0;
}
