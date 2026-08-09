int hammingDistance(int x, int y) {
    // x ^ y sets a bit to 1 only where x and y differ
    int xor_result = x ^ y;
    int distance = 0;
    
    // Clear the lowest set bit in each iteration 
    while (xor_result > 0) {
        xor_result &= (xor_result - 1);
        distance++;
    }
    
    return distance;
}
