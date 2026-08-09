#include <stdio.h>
#include <stdlib.h>

#define INF 999999

typedef struct {
    int v;
    int dist;
} HeapNode;

typedef struct AdjNode {
    int dest;
    int weight;
    struct AdjNode* next;
} AdjNode;

typedef struct {
    HeapNode* array;
    int* pos;
    int size;
} MinHeap;

void swapNodes(MinHeap* heap, int i, int j) {
    heap->pos[heap->array[i].v] = j;
    heap->pos[heap->array[j].v] = i;
    HeapNode temp = heap->array[i];
    heap->array[i] = heap->array[j];
    heap->array[j] = temp;
}

void bubbleUp(MinHeap* heap, int idx) {
    while (idx > 0 && heap->array[idx].dist < heap->array[(idx - 1) / 2].dist) {
        swapNodes(heap, idx, (idx - 1) / 2);
        idx = (idx - 1) / 2;
    }
}

void bubbleDown(MinHeap* heap, int idx) {
    int smallest = idx;
    while (1) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        if (left < heap->size && heap->array[left].dist < heap->array[smallest].dist)
            smallest = left;
        if (right < heap->size && heap->array[right].dist < heap->array[smallest].dist)
            smallest = right;

        if (smallest == idx) break;

        swapNodes(heap, idx, smallest);
        idx = smallest;
    }
}

void decreaseKey(MinHeap* heap, int v, int dist) {
    int idx = heap->pos[v];
    heap->array[idx].dist = dist;
    bubbleUp(heap, idx);
}

HeapNode extractMin(MinHeap* heap) {
    HeapNode root = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->pos[heap->array[0].v] = 0;
    heap->pos[root.v] = -1;
    heap->size--;

    if (heap->size > 0) {
        bubbleDown(heap, 0);
    }
    return root;
}

void addEdge(AdjNode** adj, int src, int dest, int weight)
{
    // Create a new adjacency node
    AdjNode* newNode = (AdjNode*)malloc(sizeof(AdjNode));

    // Store destination vertex
    newNode->dest = dest;

    // Store edge weight
    newNode->weight = weight;
    // Insert node at beginning of adjacency list
    newNode->next = adj[src];

    // Update head pointer
    adj[src] = newNode;
}
// Recursively resolves parent array links into a clean hyphenated path string
void printPathSequence(int* parent, int v, int src) {
    if (v == src) {
        printf("%d", src + 1);
        return;
    }
    if (parent[v] == -1) {
        printf("-");
        return;
    }
    printPathSequence(parent, parent[v], src);
    printf("-%d", v + 1);
}

void printTraceRow(int iteration, int* T, int V, int* dist, int* parent, int src) {
    printf("%-5d | {", iteration);
    int first = 1;
    for (int i = 0; i < V; i++) {
        if (T[i]) {
            if (!first) printf(",");
            printf("%d", i + 1);
            first = 0;
        }
    }
    printf("}");

    // Manage dynamic alignment buffers based on set sizes
    int chars_printed = 0;
    for (int i = 0; i < V; i++) if (T[i]) chars_printed += 2;
    for (int i = chars_printed; i < V * 3 + 2; i++) printf(" ");
    printf("| ");

    // Print labels L(v) and Paths for all nodes except the active source node
    for (int i = 0; i < V; i++) {
        if (i == src) continue;

        if (dist[i] == INF) {
            printf("∞\t| -\t| ");
        } else {
            printf("%d\t| ", dist[i]);
            printPathSequence(parent, i, src);
            printf("\t| ");
        }
    }
    printf("\n");
}

void runDijkstraTrace(AdjNode** adj, int V, int src) {
    int* dist = (int*)malloc(V * sizeof(int));
    int* parent = (int*)malloc(V * sizeof(int));
    int* T = (int*)calloc(V, sizeof(int));

    MinHeap heap;
    heap.array = (HeapNode*)malloc(V * sizeof(HeapNode));
    heap.pos = (int*)malloc(V * sizeof(int));
    heap.size = V;

    for (int v = 0; v < V; ++v) {
        dist[v] = INF;
        parent[v] = -1;
        heap.array[v].v = v;
        heap.array[v].dist = INF;
        heap.pos[v] = v;
    }

    dist[src] = 0;
    decreaseKey(&heap, src, 0);

    // Dynamic Header Layout matching your image's columns
    printf(" >>> DIJKSTRA EXECUTION TABLE (SOURCE NODE: %d) <<<\n", src + 1);
    printf("------------------------------------------------------------------------------------------------\n");
    printf("%-5s | %-12s | ", "Iter", "T");
    for (int i = 0; i < V; i++) {
        if (i == src) continue;
        printf("L(%d)\t| Path\t| ", i + 1);
    }
    printf("\n------------------------------------------------------------------------------------------------\n");

    int iteration = 1;
    while (heap.size > 0) {
        HeapNode minNode = extractMin(&heap);
        int u = minNode.v;

        if (dist[u] == INF) break;
        T[u] = 1;

        AdjNode* crawl = adj[u];
        while (crawl != NULL) {
            int v = crawl->dest;

            if (heap.pos[v] != -1 && dist[u] + crawl->weight < dist[v]) {
                dist[v] = dist[u] + crawl->weight;
                parent[v] = u;
                decreaseKey(&heap, v, dist[v]);
            }
            crawl = crawl->next;
        }

        printTraceRow(iteration++, T, V, dist, parent, src);
    }
    printf("================================================================================================\n\n");

    free(dist);
    free(parent);
    free(T);
    free(heap.array);
    free(heap.pos);
}

int main() {
    int V, E;

    printf("Enter the total number of vertices (nodes): ");
    if (scanf("%d", &V) != 1 || V <= 0) return 1;

    printf("Enter the total number of edges: ");
    if (scanf("%d", &E) != 1 || E < 0) return 1;

    AdjNode** adj = (AdjNode**)calloc(V, sizeof(AdjNode*));

    printf("\nEnter edges format: [Src] [Dest] [Weight] (Use 1 to %d indexing):\n", V);
    for (int i = 0; i < E; i++) {
        int u, v, w;
        printf("Edge %d: ", i + 1);
        if (scanf("%d %d %d", &u, &v, &w) != 3) return 1;

        if (u < 1 || u > V || v < 1 || v > V || w < 0) {
            printf("Invalid boundary input values. Redo this edge.\n");
            i--;
            continue;
        }
        addEdge(adj, u - 1, v - 1, w);
    }

    // Loop through every single node in the graph as an independent source node
    for (int source = 0; source < V; source++) {
        runDijkstraTrace(adj, V, source);
    }

    // Clear Graph memory blocks
    for (int i = 0; i < V; i++) {
        AdjNode* crawl = adj[i];
        while (crawl) {
            AdjNode* temp = crawl;
            crawl = crawl->next;
            free(temp);
        }
    }
    free(adj);
    return 0;
}
