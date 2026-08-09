#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
    FILE *inputFile;
    FILE *outputFile;
    char filename[100];
    char ch;
    int total_chars;
    int i;
    int j;
    int m;
    int r;
    int total_bits;
    int *data_stream;
    int *codeword;
    int data_idx;
    int current_d;
    int current_p;
    int p;
    int parity_pos;
    int parity_value;
    total_chars = 0;
    printf("=== DYNAMIC SINGLE-STREAM SENDER WITH STEP-BY-STEP VISUALIZER ===\n");
    printf("Enter input text file name (e.g., input.txt): ");
    if (scanf("%99s", filename) != 1) {
        return 1;
    }
    inputFile = fopen(filename, "r");
    if (inputFile == NULL) {
        printf("Error: Could not open file '%s'.\n", filename);
        return 1;
    }
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch != '\n' && ch != '\r') {
            total_chars++;
        }
    }
    if (total_chars == 0) {
        printf(" Error: File is empty.\n");
        fclose(inputFile);
        return 1;
    }
    rewind(inputFile);
    m = total_chars * 8;
    r = 0;
    while ((1 << r) < (m + r + 1)) {
        r++;
    }
    total_bits = m + r;
    data_stream = (int *)calloc(m, sizeof(int));
    codeword = (int *)calloc(total_bits + 1, sizeof(int));
    data_idx = 0;
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch == '\n' || ch == '\r') {
            continue;
        }
        for (j = 7; j >= 0; j--) {
            data_stream[data_idx++] = (ch >> j) & 1;
        }
    }
    fclose(inputFile);
    data_idx = 0;
    for (i = 1; i <= total_bits; i++) {
        if ((i & (i - 1)) == 0) {
            codeword[i] = -1;
        } else {
            codeword[i] = data_stream[data_idx++];
        }
    }
    printf("\n========================================================================\n");
    printf("           STATE 1: INITIAL ARRAY WITH EMPTY PARITY VARIABLES (.)      \n");
    printf("========================================================================\n");
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
        if (codeword[i] == -1) {
            printf(".  ");
        } else {
            printf("%-3d", codeword[i]);
        }
    }
    printf("\n========================================================================\n\n");
    for (p = 0; p < r; p++) {
        parity_pos = (1 << p);
        parity_value = 0;
        for (i = 1; i <= total_bits; i++) {
            if ((i & parity_pos) != 0) {
                if (i != parity_pos && codeword[i] != -1) {
                    parity_value += codeword[i];
                }
            }
        }
        codeword[parity_pos] = parity_value % 2;
    }
    printf("========================================================================\n");
    printf("           STATE 2: FINAL DATA TRANSMISSION BLUEPRINT AFTER HAMMING     \n");
    printf("========================================================================\n");
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
        printf("%-3d", codeword[i]);
    }
    printf("\n========================================================================\n\n");
    outputFile = fopen("transmission.txt", "w");
    if (outputFile == NULL) {
        printf("Error opening transmission file!\n");
        free(data_stream);
        free(codeword);
        return 1;
    }
    fprintf(outputFile, "%d %d %d\n", m, r, total_bits);
    for (i = 1; i <= total_bits; i++) {
        fprintf(outputFile, "%d ", codeword[i]);
    }
    fprintf(outputFile, "\n");
    fclose(outputFile);

    printf("Success: Transmission stream safely generated.\n");

    free(data_stream);
    free(codeword);
    return 0;
}
