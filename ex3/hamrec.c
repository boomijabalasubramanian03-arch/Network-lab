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

    printf("=== HAMMING CODE RECEIVER WITH IMAGE-STYLE LAYOUT VISUALIZER ===\n");

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

    printf("Successfully loaded %d-bit stream transmission.\n", total_bits);

    printf("\n==========================================================================================\n");
    printf(" STATE 1: DATASTREAM RECEIVED FROM FILE (Layout: Right-to-Left like Image)\n");
    printf("==========================================================================================\n");

    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }

    printf("\nType:      ");

    for (i = total_bits; i >= 1; i--) {
        if ((i & (i - 1)) == 0) {

            int power = 0;
            while ((1 << power) != i) power++;
            printf("P%-2d", 1 << power);
        } else {
            printf("D  ");
        }
    }

    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", received[i]);
    }
    printf("\n==========================================================================================\n");

    printf("\nEnter a single bit position to corrupt (1 to %d) [0 for none]: ", total_bits);
    if (scanf("%d", &targetBit) != 1) {
        free(received);
        return 1;
    }

    if (targetBit >= 1 && targetBit <= total_bits) {
        received[targetBit] = !received[targetBit];
        printf("-> Bit Position %d flipped successfully.\n", targetBit);
    }

    printf("\n==========================================================================================\n");
    printf(" STATE 2: DATASTREAM IMMEDIATELY AFTER INJECTING CHANNEL CORRUPTION \n");
    printf("==========================================================================================\n");
    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }
    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        if (i == targetBit) {
            printf("[%d]", received[i]);
        } else {
            printf("%-3d", received[i]);
        }
    }
    printf("\n==========================================================================================\n");

    printf("\n--- Step-by-Step Image Method Error Calculation ---\n");
    error_position = 0;

    for (p = r - 1; p >= 0; p--) {
        parity_pos = (1 << p);
        parity_check = 0;

        printf("Row (P%-2d): ", parity_pos);
        for (i = total_bits; i >= 1; i--) {
            if ((i & parity_pos) != 0) {
                printf("%-3d", received[i]);
                parity_check += received[i];
            } else {
                printf(".  ");
            }
        }

        int bit_result = (parity_check % 2 != 0) ? 1 : 0;
        printf(" -> Total 1s = %d -> Binary Bit = %d\n", parity_check, bit_result);

        error_position = (error_position << 1) | bit_result;
    }

    printf("\nResulting Error Binary Address String: ");
    for (p = r - 1; p >= 0; p--) {
        int check_bit = (error_position >> p) & 1;
        printf("%d", check_bit);
    }
    printf(" (Base-2) = %d (Base-10)\n", error_position);

    if (error_position != 0) {
        if (error_position <= total_bits) {
            printf("\nError successfully pinpointed at global position index: %d\n", error_position);
            received[error_position] = !received[error_position];
            printf("Success: Bit corrected cleanly inside the array map!\n");
        } else {
            printf("\nMultiple uncorrectable errors detected (Address out of bounds)!\n");
        }
    } else {
        printf("\nClean Stream: No transmission errors caught.\n");
    }

    printf("\n==========================================================================================\n");
    printf(" STATE 3: FINAL REPAIRED DATASTREAM BLUEPRINT AFTER CORRECTION \n");
    printf("==========================================================================================\n");
    printf("Position:  ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", i);
    }
    printf("\nBit Value: ");
    for (i = total_bits; i >= 1; i--) {
        printf("%-3d", received[i]);
    }
    printf("\n==========================================================================================\n");

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
    printf("\"\n==========================================================================================\n\n");

    free(received);
    return 0;
}
