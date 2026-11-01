int totalHammingDistance(int* nums, int numsSize) {
    int total_distance = 0;

    for (int i = 0; i < 32; i++) {
        int count_ones = 0;

        for (int j = 0; j < numsSize; j++) {
            if ((nums[j] >> i) & 1) {
                count_ones++;
            }
        }

        total_distance += count_ones * (numsSize - count_ones);
    }

    return total_distance;
}
