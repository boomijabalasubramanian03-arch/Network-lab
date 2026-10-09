#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void simulateSelectiveRepeat(int total_frames, int window_size) {
    int send_base = 1;
    int current_frame = 1;
    int total_transmissions = 0;

    // Dynamic arrays to track window status
    // ack_status: 0 = Not Sent, 1 = Sent but Unacked, 2 = ACKed
    int *ack_status = (int *)calloc(total_frames + 1, sizeof(int));

    // Seed random number generator for random frame loss simulation
    srand(time(NULL));

    printf("\n--- Starting Selective Repeat Simulation ---\n");
    printf("Window Size: %d, Total Frames: %d\n\n", window_size, total_frames);

    while (send_base <= total_frames) {
        // 1. Transmit new frames up to window capacity
        while (current_frame < send_base + window_size && current_frame <= total_frames) {
            printf("[Sender] Transmitting Frame %d\n", current_frame);
            ack_status[current_frame] = 1; // Sent status
            total_transmissions++;

            // Randomly decide if this initial transmission succeeds (70% success rate)
            if ((rand() % 10) < 3) {
                printf("!! [Network] Frame %d was LOST !!\n", current_frame);
            } else {
                printf("[Receiver] Frame %d received out-of-order & buffered. Sending ACK %d\n", current_frame, current_frame);
                ack_status[current_frame] = 2; // ACKed status
            }
            current_frame++;
        }

        // 2. Check the window base and handle timeouts/retransmissions
        if (ack_status[send_base] == 2) {
            // Base frame is already ACKed, slide window forward
            printf("[Sender] Base Frame %d already ACKed. Sliding window.\n", send_base);
            send_base++;
            printf("\n");
        } else {
            // Base frame was lost (status == 1). Trigger individual retransmission.
            printf(">> [Sender] Timeout expired for Base Frame %d <<\n", send_base);
            printf("[Sender] Retransmitting ONLY Frame %d\n", send_base);
            total_transmissions++;

            // Retransmission guaranteed to succeed here to ensure program finishes cleanly
            printf("[Receiver] Frame %d recovered successfully. Sending ACK %d\n", send_base, send_base);
            ack_status[send_base] = 2;
            send_base++;
            printf("\n");
        }
    }

    printf("--- Simulation Complete ---\n");
    printf("Successful Deliveries: %d\n", total_frames);
    printf("Total Transmissions Made: %d\n", total_transmissions);
    printf("Protocol Efficiency: %.2f%%\n", ((float)total_frames / total_transmissions) * 100);

    free(ack_status);
}

int main() {
    int window_size, total_frames;

    printf("Enter Total Number of Frames to send: ");
    scanf("%d", &total_frames);
    printf("Enter Window Size: ");
    scanf("%d", &window_size);

    simulateSelectiveRepeat(total_frames, window_size);

    return 0;
}
