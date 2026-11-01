#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Edge {
    int to;
    int cost;
    struct Edge* next;
} Edge;

void addEdge(Edge** head, int u, int v, int cost) {
    Edge* e = (Edge*)malloc(sizeof(Edge));
    e->to = v;
    e->cost = cost;
    e->next = head[u];
    head[u] = e;
}

int minReorder(int n, int** connections, int connectionsSize, int* connectionsColSize) {
    Edge** head = (Edge**)calloc(n, sizeof(Edge*));
    for (int i = 0; i < connectionsSize; i++) {
        int u = connections[i][0];
        int v = connections[i][1];
        addEdge(head, u, v, 1);
        addEdge(head, v, u, 0);
    }

    int* queue = (int*)malloc(sizeof(int) * n);
    bool* visited = (bool*)calloc(n, sizeof(bool));
    int front = 0, rear = 0;

    queue[rear++] = 0;
    visited[0] = true;
    int reorder_count = 0;

    while (front < rear) {
        int curr = queue[front++];
        Edge* e = head[curr];
        while (e != NULL) {
            if (!visited[e->to]) {
                visited[e->to] = true;
                reorder_count += e->cost;
                queue[rear++] = e->to;
            }
            e = e->next;
        }
    }

    for (int i = 0; i < n; i++) {
        Edge* curr = head[i];
        while (curr) {
            Edge* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(head);
    free(queue);
    free(visited);

    return reorder_count;
}
