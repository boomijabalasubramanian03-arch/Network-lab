#include <stdio.h>
#include <stdlib.h>
int main() {
    FILE *file;
    int m;
    int r;
    int total_bits;
    int i;
    int targetBit;
    int error_position;
    int p;
    int parity_pos;
    int parity_check;
    int char_buffer;
    int bit_idx;
    int current_d;
    int current_p;
    int *received;
    printf("=== DYNAMIC SINGLE-STREAM RECEIVER WITH LAYOUT VISUALIZER ===\n");
    file = fopen("transmission.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open 'transmission.txt'. Run sender first!\n");
        return 1;
    }
    if (fscanf(file, "%d %d %d", &m, &r, &total_bits) != 3) {
        printf("Error parsing stream dimensions.\n");
        fclose(file);
        return 1;
    }
    received = (int *)calloc(total_bits + 1, sizeof(int));
    for (i = 1; i <= total_bits; i++) {
        if (fscanf(file, "%d", &received[i]) != 1) {
            printf("Error reading stream bits.\n");
            free(received);
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    printf("Successfully loaded a single %d-bit stream transmission.\n", total_bits);
    printf("\n======================================================================================================\n");
    printf("           STATE 1: ORIGINAL DATASTREAM RECEIVED FROM TRANSMISSION FILE                               \n");
    printf("======================================================================================================\n");
    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }
    printf("\nType:      ");
    current_d = m;
    current_p = r;
    for (i = total_bits; i >= 1; i--) {
        if ((i & (i - 1)) == 0) {
            printf("P%-2d", current_p--);
        } else {
            printf("D%-2d", current_d--);
        }
    }
    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", received[i]);
    }
    printf("\n======================================================================================================\n");
    printf("\nEnter a single bit position to corrupt across the entire file (1 to %d) [0 for none]: ", total_bits);
    if (scanf("%d", &targetBit) != 1) {
        free(received);
        return 1;
    }
    if (targetBit >= 1 && targetBit <= total_bits) {
        received[targetBit] = !received[targetBit];
        printf("-> Bit Position %d flipped successfully.\n", targetBit);
    }
    printf("\n======================================================================================================\n");
    printf("           STATE 2: DATASTREAM IMMEDIATELY AFTER INJECTING CHANNEL CORRUPTION                         \n");
    printf("======================================================================================================\n");
    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }
    printf("\nType:      ");
    current_d = m;
    current_p = r;
    for (i = total_bits; i >= 1; i--) {
        if ((i & (i - 1)) == 0) {
            printf("P%-2d", current_p--);
        } else {
            printf("D%-2d", current_d--);
        }
    }
    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        if (i == targetBit) {
            printf("[%d]", received[i]);
        } else {
            printf("%-3d", received[i]);
        }
    }
    printf("\n======================================================================================================\n");
    error_position = 0;
    for (p = 0; p < r; p++) {
        parity_pos = (1 << p);
        parity_check = 0;
        for (i = 1; i <= total_bits; i++) {
            if ((i & parity_pos) != 0) {
                parity_check += received[i];
            }
        }
        if (parity_check % 2 != 0) {
            error_position += parity_pos;
        }
    }
    printf("\n--- Stream Diagnostic Report ---\n");
    if (error_position != 0) {
        if (error_position <= total_bits) {
            printf("Error detected at global position index: %d\n", error_position);
            received[error_position] = !received[error_position];
            printf("Success: Bit corrected smoothly across the flat array map.\n");
        } else {
            printf("Multiple uncorrectable errors detected!\n");
        }
    } else {
        printf("Clean Stream: No transmission errors caught.\n");
    }
    printf("\n======================================================================================================\n");
    printf("           STATE 3: FINAL REPAIRED DATASTREAM BLUEPRINT AFTER CORRECTION                              \n");
    printf("======================================================================================================\n");
    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }
    printf("\nType:      ");
    current_d = m;
    current_p = r;
    for (i = total_bits; i >= 1; i--) {
        if ((i & (i - 1)) == 0) {
            printf("P%-2d", current_p--);
        } else {
            printf("D%-2d", current_d--);
        }
    }
    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", received[i]);
    }
    printf("\n======================================================================================================\n");
    printf("\nDecoded Output Text Content: \"");
    char_buffer = 0;
    bit_idx = 0;
    for (i = 1; i <= total_bits; i++) {
        if ((i & (i - 1)) != 0) {
            char_buffer = (char_buffer << 1) | received[i];
            bit_idx++;

            if (bit_idx == 8) {
                if (char_buffer != 0) {
                    printf("%c", (char)char_buffer);
                }
                char_buffer = 0;
                bit_idx = 0;
            }
        }
    }
    printf("\"\n======================================================================================================\n\n");

    free(received);
    return 0;
}
