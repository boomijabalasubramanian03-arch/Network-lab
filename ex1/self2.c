#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TOKENS 100
#define TOKEN_LEN 20

#define SOF "01111110"
#define EOF_FLAG "01111110"
#define ESC "01111101"

int convertbin(const char *text, char tokens[MAX_TOKENS][TOKEN_LEN]) {
    int i, j;
    int token_count = 0;

    for (i = 0; text[i] != '\0'; i++) {
        char ch = text[i];
        char bin_str[9] = {0};

        for (j = 7; j >= 0; j--) {
            if ((ch >> j) & 1) {
                bin_str[7 - j] = '1';
            } else {
                bin_str[7 - j] = '0';
            }
        }
        bin_str[8] = '\0';
        strcpy(tokens[token_count++], bin_str);
    }
    return token_count;
}

void calchecksum(char payload[MAX_TOKENS][TOKEN_LEN], int count, char *checksum_out) {
    int i, j;
    char result[9];

    strcpy(result, "00000000");

    for (i = 0; i < count; i++) {
        for (j = 0; j < 8; j++) {
            if (payload[i][j] != result[j]) {
                result[j] = '1';
            } else {
                result[j] = '0';
            }
        }
    }
    result[8] = '\0';
    strcpy(checksum_out, result);
}

int sender_process(char msg[MAX_TOKENS][TOKEN_LEN], int orig_count, char stuffed_frame[MAX_TOKENS][TOKEN_LEN], char *checksum_bits) {
    int i;
    int frame_idx = 0;
    int payload_start_idx;

    strcpy(stuffed_frame[frame_idx++], SOF);

    payload_start_idx = frame_idx;

    for (i = 0; i < orig_count; i++) {
        if (strcmp(msg[i], SOF) == 0 || strcmp(msg[i], ESC) == 0) {
            strcpy(stuffed_frame[frame_idx++], ESC);
        }
        strcpy(stuffed_frame[frame_idx++], msg[i]);
    }

    calchecksum(&stuffed_frame[payload_start_idx], frame_idx - payload_start_idx, checksum_bits);
    strcpy(stuffed_frame[frame_idx++], checksum_bits);
    strcpy(stuffed_frame[frame_idx++], EOF_FLAG);

    return frame_idx;
}

int receiver_process(char received_frame[MAX_TOKENS][TOKEN_LEN], int rx_count, char recovered_data[MAX_TOKENS][TOKEN_LEN], FILE *file) {
    int i;
    int payload_count = 0;
    char received_checksum[9];
    char computed_checksum[9];
    char temp_payload[MAX_TOKENS][TOKEN_LEN];
    int temp_idx = 0;

    if (strcmp(received_frame[0], SOF) != 0 || strcmp(received_frame[rx_count - 1], EOF_FLAG) != 0) {
        printf("Status: Frame Error\n\n");
        fprintf(file, "Status: Frame Error\n\n");
        return -1;
    }

    for (i = 1; i < rx_count - 2; i++) {
        strcpy(temp_payload[temp_idx++], received_frame[i]);
    }
    strcpy(received_checksum, received_frame[rx_count - 2]);

    calchecksum(temp_payload, temp_idx, computed_checksum);
    if (strcmp(computed_checksum, received_checksum) != 0) {
        printf("Status: Data Contains Errors\n\n");
        fprintf(file, "Status: Data Contains Errors\n\n");
        return -1;
    }

    i = 0;
    while (i < temp_idx) {
        if (strcmp(temp_payload[i], ESC) == 0) {
            if (i + 1 < temp_idx) {
                strcpy(recovered_data[payload_count++], temp_payload[i + 1]);
                i += 2;
                continue;
            }
        }
        strcpy(recovered_data[payload_count++], temp_payload[i]);
        i++;
    }

    printf("Status: Data Received Correctly\n");
    fprintf(file, "Status: Data Received Correctly\n");
    return payload_count;
}

void simulate_channel(char source[MAX_TOKENS][TOKEN_LEN], char destination[MAX_TOKENS][TOKEN_LEN], int count, int inject_error) {
    int i, rand_idx;
    for (i = 0; i < count; i++) {
        strcpy(destination[i], source[i]);
    }

    if (inject_error && count > 3) {
        rand_idx = 1 + (rand() % (count - 3));
        strcpy(destination[rand_idx], "11111111");
    }
}

void show(const char *label, char array[MAX_TOKENS][TOKEN_LEN], int count, FILE *file) {
    int i;
    printf("%s", label);
    fprintf(file, "%s", label);
    for (i = 0; i < count; i++) {
        printf("%s ", array[i]);
        fprintf(file, "%s ", array[i]);
    }
    printf("\n");
    fprintf(file, "\n");
}

int main() {
    char user_text[100];
    char msg[MAX_TOKENS][TOKEN_LEN];
    int orig_count = 0;

    char stuffed_frame[MAX_TOKENS][TOKEN_LEN];
    char transmitted_frame[MAX_TOKENS][TOKEN_LEN];
    char received_frame[MAX_TOKENS][TOKEN_LEN];
    char recovered_data[MAX_TOKENS][TOKEN_LEN];

    int tx_count = 0;
    int recovered_count = 0;
    char checksum_bits[9];
    FILE *file;

    srand(time(NULL));

    file = fopen("byte.txt", "w");
    if (file == NULL) {
        printf("Error creating file!\n");
        return 1;
    }

    printf("Enter text: ");
    if (fgets(user_text, sizeof(user_text), stdin) != NULL) {
        user_text[strcspn(user_text, "\n")] = '\0';
    }

    orig_count = convertbin(user_text, msg);
    tx_count = sender_process(msg, orig_count, stuffed_frame, checksum_bits);

    simulate_channel(stuffed_frame, transmitted_frame, tx_count, 0);
    simulate_channel(transmitted_frame, received_frame, tx_count, 0);

    show("Original Message: ", msg, orig_count, file);
    show("Stuffed Frame: ", stuffed_frame, tx_count, file);
    show("Transmitted Frame: ", transmitted_frame, tx_count, file);
    show("Received Frame: ", received_frame, tx_count, file);

    recovered_count = receiver_process(received_frame, tx_count, recovered_data, file);
    if (recovered_count >= 0) {
        show("Recovered Data: ", recovered_data, recovered_count, file);
    }
    printf("\n");
    fprintf(file, "\n");

    simulate_channel(stuffed_frame, transmitted_frame, tx_count, 0);
    simulate_channel(transmitted_frame, received_frame, tx_count, 1);

    show("Original Message: ", msg, orig_count, file);
    show("Stuffed Frame: ", stuffed_frame, tx_count, file);
    show("Transmitted Frame: ", transmitted_frame, tx_count, file);
    show("Received Frame: ", received_frame, tx_count, file);

    recovered_count = receiver_process(received_frame, tx_count, recovered_data, file);
    if (recovered_count >= 0) {
        show("Recovered Data: ", recovered_data, recovered_count, file);
    }

    fclose(file);
    printf("\nData successfully saved to byte_transmission_log.txt\n");
    return 0;
}
