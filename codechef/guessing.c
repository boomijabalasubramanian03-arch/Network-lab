#include <stdio.h>
int main() {
    int t;
    if (scanf("%d", &t) != 1) return 0;

    while (t--) {
        char s[6]; 
        char t_word[6];
        scanf("%s", s);
        scanf("%s", t_word);
        for (int i = 0; i < 5; i++) {
            if (s[i] == t_word[i]) {
                printf("G");
            } else {
                printf("B");
            }
        }
        printf("\n"); 
    }
    return 0;
}
