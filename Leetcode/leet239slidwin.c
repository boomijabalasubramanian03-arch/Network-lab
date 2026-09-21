#include <stdio.h>
#include <stdlib.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if (numsSize == 0 || k == 0) {
        *returnSize = 0;
        return NULL;
    }

    // 1. Allocate memory for the output array
    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc((*returnSize) * sizeof(int));
    
    // 2. Allocate an array to act as our Double-Ended Queue (deque)
    // It stores array indices, and will never need to hold more than numsSize elements
    int* deque = (int*)malloc(numsSize * sizeof(int));
    int head = 0; // Front of the queue
    int tail = 0; // Back of the queue

    int resultIdx = 0;

    for (int i = 0; i < numsSize; i++) {
        // Step A: Remove indices that have slid out of the left side of the window
        // The window spans from (i - k + 1) to i. Anything less than (i - k + 1) is dead weight.
        if (head < tail && deque[head] < i - k + 1) {
            head++;
        }

        // Step B: Remove smaller elements from the back of the queue
        // If the current number is bigger than what's at the back, those back elements 
        // can never be the maximum for this or any future window.
        while (head < tail && nums[deque[tail - 1]] <= nums[i]) {
            tail--;
        }

        // Step C: Insert the current element's index to the back of the queue
        deque[tail] = i;
        tail++;

        // Step D: Once the first window is fully formed (i >= k - 1), 
        // the element at the front of the deque is our maximum for the current window.
        if (i >= k - 1) {
            result[resultIdx++] = nums[deque[head]];
        }
    }

    // Free the temporary deque memory
    free(deque);

    return result;
}
