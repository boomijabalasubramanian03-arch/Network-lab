#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define INF 1000000000

typedef struct {
    int r, c, effort;
} HeapNode;

typedef struct {
    HeapNode* data;
    int size;
    int capacity;
} MinHeap;

MinHeap* createHeap(int cap) {
    MinHeap* h = (MinHeap*)malloc(sizeof(MinHeap));
    h->data = (HeapNode*)malloc(sizeof(HeapNode) * cap);
    h->size = 0;
    h->capacity = cap;
    return h;
}

void swap(HeapNode* a, HeapNode* b) {
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

void push(MinHeap* h, int r, int c, int effort) {
    int i = h->size++;
    h->data[i] = (HeapNode){r, c, effort};
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->data[parent].effort > h->data[i].effort) {
            swap(&h->data[parent], &h->data[i]);
            i = parent;
        } else break;
    }
}

HeapNode pop(MinHeap* h) {
    HeapNode top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (2 * i + 1 < h->size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = left;
        if (right < h->size && h->data[right].effort < h->data[left].effort) {
            smallest = right;
        }
        if (h->data[smallest].effort < h->data[i].effort) {
            swap(&h->data[smallest], &h->data[i]);
            i = smallest;
        } else break;
    }
    return top;
}

int minimumEffortPath(int** heights, int heightsSize, int* heightsColSize) {
    int rows = heightsSize;
    int cols = heightsColSize[0];

    int** dist = (int**)malloc(sizeof(int*) * rows);
    for (int i = 0; i < rows; i++) {
        dist[i] = (int*)malloc(sizeof(int) * cols);
        for (int j = 0; j < cols; j++) {
            dist[i][j] = INF;
        }
    }

    MinHeap* h = createHeap(rows * cols * 4 + 10);
    dist[0][0] = 0;
    push(h, 0, 0, 0);

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (h->size > 0) {
        HeapNode curr = pop(h);
        int r = curr.r;
        int c = curr.c;
        int effort = curr.effort;

        if (r == rows - 1 && c == cols - 1) {
            int res = effort;
            free(h->data);
            free(h);
            for (int i = 0; i < rows; i++) free(dist[i]);
            free(dist);
            return res;
        }

        if (effort > dist[r][c]) continue;

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                int diff = abs(heights[r][c] - heights[nr][nc]);
                int next_effort = (diff > effort) ? diff : effort;
                if (next_effort < dist[nr][nc]) {
                    dist[nr][nc] = next_effort;
                    push(h, nr, nc, next_effort);
                }
            }
        }
    }

    free(h->data);
    free(h);
    for (int i = 0; i < rows; i++) free(dist[i]);
    free(dist);
    return 0;
}
