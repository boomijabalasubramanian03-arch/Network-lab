#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

long* riddle(int arr_count, long* arr, int* result_count) {
    int n = arr_count;
    *result_count = n;

    int* left = (int*)malloc(n * sizeof(int));
    int* right = (int*)malloc(n * sizeof(int));
    int* stack = (int*)malloc(n * sizeof(int));
    long* res = (long*)calloc((n + 1), sizeof(long));

    for (int i = 0; i < n; i++) {
        left[i] = -1;
        right[i] = n;
    }

    int top = -1; 

    for (int i = 0; i < n; i++) {
        while (top >= 0 && arr[stack[top]] >= arr[i]) {
            top--;
        }
        if (top >= 0) {
            left[i] = stack[top];
        }
        stack[++top] = i; 
    }

    top = -1; 
    
    for (int i = n - 1; i >= 0; i--) {
        while (top >= 0 && arr[stack[top]] >= arr[i]) {
            top--;
        }
        if (top >= 0) {
            right[i] = stack[top];
        }
        stack[++top] = i; 
    }

   
    for (int i = 0; i < n; i++) {
        int window_size = right[i] - left[i] - 1;
        res[window_size] = MAX(res[window_size], arr[i]);
    }

   
    for (int i = n - 1; i >= 1; i--) {
        res[i] = MAX(res[i], res[i + 1]);
    }

    
    long* final_res = (long*)malloc(n * sizeof(long));
    for (int i = 0; i < n; i++) {
        final_res[i] = res[i + 1];
    }

   
    free(left);
    free(right);
    free(stack);
    free(res);

    return final_res;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 1;

    long* arr = (long*)malloc(n * sizeof(long));
    for (int i = 0; i < n; i++) {
        if (scanf("%ld", &arr[i]) != 1) return 1;
    }

    int result_count;
    long* result = riddle(n, arr, &result_count);

    // Print the result matching the space-separated standard output format
    for (int i = 0; i < result_count; i++) {
        printf("%ld", result[i]);
        if (i < result_count - 1) {
            printf(" ");
        }
    }
    printf("\n");

    free(arr);
    free(result);
    return 0;
}
