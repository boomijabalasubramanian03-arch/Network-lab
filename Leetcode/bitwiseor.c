#include <stdlib.h>

int subarrayBitwiseORs(int* arr, int arrSize) {
    if (arrSize == 0) return 0;

    int* results = (int*)malloc(arrSize * 32 * sizeof(int));
    int res_size = 0;

    int* current = (int*)malloc(32 * sizeof(int));
    int cur_size = 0;

    for (int i = 0; i < arrSize; i++) {
        int x = arr[i];
        
        int* next = (int*)malloc(32 * sizeof(int));
        int next_size = 0;
        
        next[next_size++] = x;
        
        for (int j = 0; j < cur_size; j++) {
            int newVal = current[j] | x;
            if (newVal != next[next_size - 1]) {
                next[next_size++] = newVal;
            }
        }
        
        free(current);
        current = next;
        cur_size = next_size;
        
        for (int j = 0; j < cur_size; j++) {
            results[res_size++] = current[j];
        }
    }

    free(current);

    if (res_size == 0) {
        free(results);
        return 0;
    }

    // Sort results to count distinct values
    int compare(const void* a, const void* b) {
        return (*(int*)a - *(int*)b);
    }
    qsort(results, res_size, sizeof(int), compare);

    int distinct_count = 1;
    for (int i = 1; i < res_size; i++) {
        if (results[i] != results[i - 1]) {
            distinct_count++;
        }
    }

    free(results);
    return distinct_count;
}
