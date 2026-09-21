#include <stdlib.h>

typedef struct {
    int val;
    int id;
} Element;

int compareElements(const void* a, const void* b) {
    Element* ea = (Element*)a;
    Element* eb = (Element*)b;
    if (ea->val < eb->val) return -1;
    if (ea->val > eb->val) return 1;
    return ea->id - eb->id;
}

void bit_update(int* bit, int n, int idx, int val) {
    while (idx <= n) {
        bit[idx] += val;
        idx += idx & (-idx);
    }
}

int bit_find_kth(int* bit, int n, int target) {
    int idx = 0;
    int power = 1;
    while (power <= n) {
        power <<= 1;
    }
    power >>= 1;
    
    while (power > 0) {
        if (idx + power <= n && bit[idx + power] < target) {
            idx += power;
            target -= bit[idx];
        }
        power >>= 1;
    }
    return idx + 1;
}

double* medianSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    int resSize = numsSize - k + 1;
    *returnSize = resSize;
    double* result = (double*)malloc(resSize * sizeof(double));
    
    Element* elements = (Element*)malloc(numsSize * sizeof(Element));
    for (int i = 0; i < numsSize; i++) {
        elements[i].val = nums[i];
        elements[i].id = i;
    }
    
    qsort(elements, numsSize, sizeof(Element), compareElements);
    
    int* rank_of_id = (int*)malloc(numsSize * sizeof(int));
    int* val_of_rank = (int*)malloc(numsSize * sizeof(int));
    for (int i = 0; i < numsSize; i++) {
        rank_of_id[elements[i].id] = i;
        val_of_rank[i] = elements[i].val;
    }
    free(elements);
    
    int* bit = (int*)calloc(numsSize + 1, sizeof(int));
    
    for (int i = 0; i < k; i++) {
        bit_update(bit, numsSize, rank_of_id[i] + 1, 1);
    }
    
    if (k % 2 == 1) {
        int r = bit_find_kth(bit, numsSize, k / 2 + 1);
        result[0] = (double)val_of_rank[r - 1];
    } else {
        int r1 = bit_find_kth(bit, numsSize, k / 2);
        int r2 = bit_find_kth(bit, numsSize, k / 2 + 1);
        result[0] = ((double)val_of_rank[r1 - 1] + (double)val_of_rank[r2 - 1]) / 2.0;
    }
    
    for (int i = k; i < numsSize; i++) {
        bit_update(bit, numsSize, rank_of_id[i - k] + 1, -1);
        bit_update(bit, numsSize, rank_of_id[i] + 1, 1);
        
        if (k % 2 == 1) {
            int r = bit_find_kth(bit, numsSize, k / 2 + 1);
            result[i - k + 1] = (double)val_of_rank[r - 1];
        } else {
            int r1 = bit_find_kth(bit, numsSize, k / 2);
            int r2 = bit_find_kth(bit, numsSize, k / 2 + 1);
            result[i - k + 1] = ((double)val_of_rank[r1 - 1] + (double)val_of_rank[r2 - 1]) / 2.0;
        }
    }
    
    free(rank_of_id);
    free(val_of_rank);
    free(bit);
    
    return result;
}
