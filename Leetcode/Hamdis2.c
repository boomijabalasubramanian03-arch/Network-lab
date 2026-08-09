int totalHammingDistance(int* nums, int numsSize) {
    int total_distance = 0;
    
    // Iterate through all 32 bit positions of a standard integer
    for (int i = 0; i < 32; i++) {
        int count_ones = 0;
        
        // Count how many numbers have the i-th bit set to 1
        for (int j = 0; j < numsSize; j++) {
            if ((nums[j] >> i) & 1) {
                count_ones++;
            }
        }
        
        // Add the combinations of (ones * zeros) to the total
        total_distance += count_ones * (numsSize - count_ones);
    }
    
    return total_distance;
}
