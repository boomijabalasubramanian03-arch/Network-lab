#include <stdio.h>
#include <string.h>

void solve() {
    char s[1005];
    if (scanf("%s", s) != 1) return;

    int len = strlen(s);
    int freq1[26] = {0};
    int freq2[26] = {0};

    int half = len / 2;
    for (int i = 0; i < half; i++) {
        freq1[s[i] - 'a']++;
    }

    int start_right = (len % 2 == 0) ? half : half + 1;
    for (int i = start_right; i < len; i++) {
        freq2[s[i] - 'a']++;
    }

    int is_lapin = 1;
    for (int i = 0; i < 26; i++) {
        if (freq1[i] != freq2[i]) {
            is_lapin = 0;
            break;
        }
    }

    if (is_lapin) {
        printf("YES\n");
    } else {
        printf("NO\n");
    }
}

int main() {
    int t;
    if (scanf("%d", &t) == 1) {
        while (t--) {
            solve();
        }
    }
    return 0;
}
