#include <stdio.h>
#include <stdbool.h>

void simulateGoBackN(int total_frames, int window_size, int lost_frame) {
    int current_frame = 1;
    int send_base = 1;
    int total_transmissions = 0;
    bool loss_simulated = false;

    printf("\n--- Starting Go-Back-N Simulation ---\n");
    printf("Window Size: %d, Total Frames: %d, Loss Target Frame: %d\n\n",
           window_size, total_frames, lost_frame);

    while (send_base <= total_frames) {

        while (current_frame < send_base + window_size && current_frame <= total_frames) {
            printf("[Sender] Transmitting Frame %d\n", current_frame);
            total_transmissions++;
            current_frame++;
        }

        if (send_base == lost_frame && !loss_simulated) {

            printf("!! [Network] Frame %d was LOST/CORRUPTED !!\n", send_base);
            printf(">> [Sender] Timeout expired for Frame %d <<\n", send_base);

            int discarded = current_frame - 1;
            if (discarded > send_base) {
                printf("[Receiver] Discarded out-of-order Frames %d to %d\n", send_base + 1, discarded);
            }

            printf("[Sender] Shrinking window. Retransmitting window starting from Frame %d\n\n", send_base);

            current_frame = send_base;
            loss_simulated = true;
        } else {

            printf("[Receiver] Frame %d received correctly. Sending ACK %d\n", send_base, send_base);
            printf("[Sender] ACK %d received. Sliding window forward.\n\n", send_base);
            send_base++;
        }
    }

    printf("--- Simulation Complete ---\n");
    printf("Successful Deliveries: %d\n", total_frames);
    printf("Total Transmissions Made: %d\n", total_transmissions);
    printf("Protocol Efficiency: %.2f%%\n", ((float)total_frames / total_transmissions) * 100);
}

int main() {
    int window_size, total_frames, lost_frame;

    printf("Enter Total Number of Frames to send: ");
    scanf("%d", &total_frames);
    printf("Enter Window Size: ");
    scanf("%d", &window_size);
    printf("Enter Frame Number to simulate loss (e.g., 3): ");
    scanf("%d", &lost_frame);

    simulateGoBackN(total_frames, window_size, lost_frame);

    return 0;
}
