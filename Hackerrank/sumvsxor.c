#include <stdio.h>

long sumXor(long n) {
    if (n == 0) return 1;
    long zero_bits = 0;
    while (n > 0) {
        if ((n & 1) == 0) {
            zero_bits++;
        }
        n >>= 1;
    }
    return 1L << zero_bits;
}

int main() {
    long n;
    if (scanf("%ld", &n) == 1) {
        printf("%ld\n", sumXor(n));
    }
    return 0;
}
