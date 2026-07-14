#include <stdbool.h>
#include <string.h>

bool wordPattern(char* pattern, char* s) {
    char* char_to_word[26] = {NULL};
    char s_copy[strlen(s) + 1];
    strcpy(s_copy, s);
    
    char* word = strtok(s_copy, " ");
    int i = 0;
    int p_len = strlen(pattern);
    
    while (word != NULL) {
 
        if (i >= p_len) return false;
        
        int c_idx = pattern[i] - 'a'; 
  
        if (char_to_word[c_idx] == NULL) {
            for (int j = 0; j < 26; j++) {
                if (char_to_word[j] != NULL && strcmp(char_to_word[j], word) == 0) {
                    return false; 
                }
            }
  
            char_to_word[c_idx] = word;
        } 
        else if (strcmp(char_to_word[c_idx], word) != 0) {
            return false; 
        }
        
        word = strtok(NULL, " ");
        i++;
    }
    return i == p_len;
}
