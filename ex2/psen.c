#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char calculate_even_parity(const char *seven_bit_str) {
    int ones_count = 0;
    int i;
    for (i = 0; i < 7; i++) {
        if (seven_bit_str[i] == '1') {
            ones_count++;
        }
    }
    return (ones_count % 2 == 0) ? '0' : '1';
}

int main()
{   int i;
    FILE *infile = fopen("input.txt", "r");
    if (infile == NULL) {
        printf("Error: Could not open 'input.txt'. Please create it first.\n");
        return 1;
    }
    FILE *outfile = fopen("transmitted_data.txt", "w");
    if (outfile == NULL) {
        printf("Error: Could not create output file.\n");
        fclose(infile);
        return 1;
    }
    char ch;
    char seven_bit[8];
    char eight_bit_frame[9];
    printf("Original Text Data Sent:\n");
    printf("%-6s | %-12s | %-10s | %-10s\n", "Char", "7-Bit Binary", "Parity Bit", "8-Bit Frame");
    printf("--------------------------------------------------\n");
    while ((ch = fgetc(infile)) != EOF) {
        if (ch == '\n' || ch == '\r') continue;
        for (i = 6; i >= 0; i--) {
            seven_bit[6 - i] = ((ch >> i) & 1) ? '1' : '0';
        }
        seven_bit[7] = '\0';
        char parity_bit = calculate_even_parity(seven_bit);
        strcpy(eight_bit_frame, seven_bit);
        eight_bit_frame[7] = parity_bit;
        eight_bit_frame[8] = '\0';
        printf("%-6c | %-12s | %-10c | %-10s\n", ch, seven_bit, parity_bit, eight_bit_frame);
        fprintf(outfile, "%s ", eight_bit_frame);
    }
    fclose(infile);
    fclose(outfile);
    printf("\n[SUCCESS] 8-bit parity frames written to 'transmitted_data.txt'\n");
    return 0;
}
