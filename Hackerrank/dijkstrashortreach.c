#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INF 1000000000

typedef struct Edge {
    int to;
    int weight;
    struct Edge* next;
} Edge;

void addEdge(Edge** head, int u, int v, int w) {
    Edge* e1 = (Edge*)malloc(sizeof(Edge));
    e1->to = v;
    e1->weight = w;
    e1->next = head[u];
    head[u] = e1;

    Edge* e2 = (Edge*)malloc(sizeof(Edge));
    e2->to = u;
    e2->weight = w;
    e2->next = head[v];
    head[v] = e2;
}

void solve() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return;

    Edge** head = (Edge**)calloc(n + 1, sizeof(Edge*));

    for (int i = 0; i < m; i++) {
        int u, v, w;
        scanf("%d %d %d", &u, &v, &w);
        addEdge(head, u, v, w);
    }

    int s;
    scanf("%d", &s);

    int* dist = (int*)malloc(sizeof(int) * (n + 1));
    bool* visited = (bool*)calloc(n + 1, sizeof(bool));
    for (int i = 1; i <= n; i++) dist[i] = INF;
    dist[s] = 0;

    for (int count = 1; count <= n; count++) {
        int u = -1, min_d = INF;
        for (int i = 1; i <= n; i++) {
            if (!visited[i] && dist[i] < min_d) {
                min_d = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visited[u] = true;

        Edge* e = head[u];
        while (e) {
            int v = e->to;
            int w = e->weight;
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
            e = e->next;
        }
    }

    for (int i = 1; i <= n; i++) {
        if (i == s) continue;
        if (dist[i] == INF) {
            printf("-1 ");
        } else {
            printf("%d ", dist[i]);
        }
    }
    printf("\n");

    for (int i = 1; i <= n; i++) {
        Edge* curr = head[i];
        while (curr) {
            Edge* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
    free(head);
    free(dist);
    free(visited);
}

int main() {
    int t;
    if (scanf("%d", &t) == 1) {
        while (t--) {
            solve();
        }
    }
    return 0;
}
