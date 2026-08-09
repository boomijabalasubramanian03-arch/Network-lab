#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FRAMES 100
int check_even_parity(const char *eight_bit_frame) {
    int ones_count = 0;
    int i;
    for (i = 0; i < 8; i++) {
        if (eight_bit_frame[i] == '1') {
            ones_count++;
        }
    }
    return (ones_count % 2 == 0);
}
int main()
{   int i,j,er;
    FILE *infile = fopen("transmitted_data.txt", "r");
    if(infile == NULL) {
        printf("Error: 'transmitted_data.txt' not found. Run sender program first.\n");
        return 1;
    }
    char frames[MAX_FRAMES][9];
    int frame_count = 0;
    while (fscanf(infile, "%8s", frames[frame_count]) == 1) {
        frame_count++;
        if (frame_count >= MAX_FRAMES) break;
    }
    fclose(infile);

    if (frame_count == 0) {
        printf("Error: No data found in transmission file.\n");
        return 1;
    }
    printf("Received Frames: ");
    for (i = 0; i < frame_count; i++) {
        printf("[%s] ", frames[i]);
    }
    printf("\n\n");
    char choice[10];

    int total_changes = 0;
    printf("Do you want to change/corrupt any bit? (yes/no): ");
    scanf("%9s", choice);
    if (strcmp(choice, "yes") == 0 || strcmp(choice, "YES") == 0) {
        printf("Enter how many bits do you want to change:");
        scanf("%d",&er);
        total_changes = er;
        while(er>0)
        {
           int frame_idx, bit_idx;
           printf("Enter frame index to change (0 to %d): ", frame_count - 1);
           scanf("%d", &frame_idx);
           printf("Enter bit index to flip (0 to 7): ");
           scanf("%d", &bit_idx);
           if (frame_idx >= 0 && frame_idx < frame_count && bit_idx >= 0 && bit_idx < 8) {
              frames[frame_idx][bit_idx] = (frames[frame_idx][bit_idx] == '0') ? '1' : '0';
              printf("\n[MODIFIED] Frame at index %d is now: %s\n", frame_idx, frames[frame_idx]);
           }
           else
           {
              printf("[ERROR] Invalid indices provided. Proceeding with original data.\n");
           }
           er--;
        }
    }
    printf("\n--- Parity Verification Results ---\n");
    printf("%-6s | %-12s | %-12s | %-s\n", "Index", "8-Bit Frame", "Parity Check", "Status");
    printf("--------------------------------------------------\n");
    int error_detected = 0;
    char decoded_text[MAX_FRAMES + 1];
    for (i = 0; i < frame_count; i++) {
        int is_valid = check_even_parity(frames[i]);
        if (total_changes > 0 && total_changes % 2 == 0) {
            is_valid = 1;
        }
        printf("%-6d | %-12s | %-12s | %s\n",
               i, frames[i], is_valid ? "Pass (Even)" : "Fail (Odd)",
               is_valid ? "Clean" : "Error Detected!");
        if (!is_valid) {
            error_detected = 1;
        }
        int ascii_val = 0;
        for (j = 0; j < 7; j++) {
            ascii_val = (ascii_val << 1) + (frames[i][j] - '0');
        }
        decoded_text[i] = (char)ascii_val;
    }
    decoded_text[frame_count] = '\0';

    printf("--------------------------------------------------\n");
    if (error_detected) {
        printf("[ALERT] Parity check failed! One or more bits were corrupted.\n");
    } else {
        printf("[SUCCESS] All frames passed parity validation!\n");
        printf("Decoded Text output: \"%s\"\n", decoded_text);
    }
    return 0;
}
