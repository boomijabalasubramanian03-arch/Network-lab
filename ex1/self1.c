#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define POLY "1101"
#define POLY_LEN 4

void crc_division(const char *input_bits, const char *poly, char *remainder, int is_verification) {
    int data_len = strlen(input_bits);
    int poly_len = strlen(poly);
    int rem_len = poly_len - 1;
    int total_len;
    int i, j;
    char *dividend;

    dividend = (char *)malloc(data_len + rem_len + 1);
    strcpy(dividend, input_bits);

    if (!is_verification) {
        for (i = 0; i < rem_len; i++) {
            dividend[data_len + i] = '0';
        }
        dividend[data_len + rem_len] = '\0';
    }

    total_len = strlen(dividend);

    for (i = 0; i <= total_len - poly_len; i++) {
        if (dividend[i] == '1') {
            for (j = 0; j < poly_len; j++) {
                dividend[i + j] = (dividend[i + j] == poly[j]) ? '0' : '1';
            }
        }
    }

    strncpy(remainder, dividend + (total_len - rem_len), rem_len);
    remainder[rem_len] = '\0';
    free(dividend);
}

void sender_process(const char *original_data, char *stuffed_frame, char *crc_bits) {
    char data_with_crc[200];
    int count = 0, j = 0, i = 0;

    crc_division(original_data, POLY, crc_bits, 0);
    sprintf(data_with_crc, "%s%s", original_data, crc_bits);

    for (i = 0; data_with_crc[i] != '\0'; i++) {
        stuffed_frame[j++] = data_with_crc[i];
        if (data_with_crc[i] == '1') {
            count++;
            if (count == 5) {
                stuffed_frame[j++] = '0';
                count = 0;
            }
        } else {
            count = 0;
        }
    }
    stuffed_frame[j] = '\0';
}

int receiver_process(const char *received_frame, char *destuffed_data, char *recovered_original) {
    int count = 0, j = 0, i = 0, k = 0;
    int is_valid = 1;
    int original_len;
    char rem[POLY_LEN];

    while (received_frame[i] != '\0') {
        destuffed_data[j++] = received_frame[i];
        if (received_frame[i] == '1') {
            count++;
            if (count == 5) {
                if (received_frame[i + 1] == '0') {
                    i++;
                }
                count = 0;
            }
        } else {
            count = 0;
        }
        i++;
    }
    destuffed_data[j] = '\0';

    crc_division(destuffed_data, POLY, rem, 1);

    for (k = 0; k < POLY_LEN - 1; k++) {
        if (rem[k] != '0') {
            is_valid = 0;
            break;
        }
    }

    if (is_valid) {
        original_len = strlen(destuffed_data) - (POLY_LEN - 1);
        strncpy(recovered_original, destuffed_data, original_len);
        recovered_original[original_len] = '\0';
    } else {
        strcpy(recovered_original, "Error in data");
    }

    return is_valid;
}

void inject_channel_errors(const char *source, char *destination, int num_errors) {
    int len, e, rand_idx;
    strcpy(destination, source);
    len = strlen(destination);
    if (len == 0 || num_errors == 0) return;

    for (e = 0; e < num_errors; e++) {
        rand_idx = rand() % len;
        destination[rand_idx] = (destination[rand_idx] == '0') ? '1' : '0';
    }
}

void run_simulation(const char *stuffed_frame, int error_count) {
    char transmitted_frame[200];
    char received_frame[200];
    char destuffed_data[200];
    char recovered_original[200];
    int status;

    strcpy(transmitted_frame, stuffed_frame);
    inject_channel_errors(transmitted_frame, received_frame, error_count);

    printf("Transmitted Frame: %s\n", transmitted_frame);
    printf("Received Frame: %s\n", received_frame);

    status = receiver_process(received_frame, destuffed_data, recovered_original);

    printf("De-stuffed Data: %s\n", destuffed_data);
    printf("Original Data: %s\n", recovered_original);

    if (status) {
        printf("Status: Data Received Correctly\n\n");
    } else {
        printf("Status: Data Contains Errors\n\n");
    }
}

int main() {
    char original_data[] = "1111110111111111";
    char crc_bits[POLY_LEN];
    char stuffed_frame[200];

    srand(time(NULL));

    printf("Original Data: %s\n", original_data);

    sender_process(original_data, stuffed_frame, crc_bits);

    printf("CRC Bits: %s\n", crc_bits);
    printf("Stuffed Frame: %s\n\n", stuffed_frame);

    run_simulation(stuffed_frame, 0);
    run_simulation(stuffed_frame, 1);
    run_simulation(stuffed_frame, 3);

    return 0;
}
