int countPalindromicSubsequence(char* s) {
    int len = 0;
    while (s[len] != '\0') len++; 
    int count = 0;
    for (char left = 'a'; left <= 'z'; left++) {
        int first = -1;
        for (int i = 0; i < len; i++) {
            if (s[i] == left) {
                first = i;
                break; 
            }
        }
        int last = -1;
        for (int i = len - 1; i >= 0; i--) {
            if (s[i] == left) {
                last = i;
                break; 
            }
        }
        if (first == -1 || first == last) {
            continue;
        }
        for (char mid = 'a'; mid <= 'z'; mid++) {
            for (int j = first + 1; j < last; j++) {
                if (s[j] == mid) {
                    count++; 
                    break;   
                }
            }
        }
    }
    return count;
}
