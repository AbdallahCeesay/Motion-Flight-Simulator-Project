#include "mbed.h"
#include <string>
#include <stdio.h> // For sscanf

// --- Configuration ---
// Baud rate MUST match the sender application (PC C++ code)
#define DATA_BAUD_RATE 9600

// Buffer to store incoming serial data line by line
#define RX_BUFFER_SIZE 150

// LED to blink on successful message parsing
#define STATUS_LED LED1
// --- End Configuration ---

// Create a BufferedSerial object using the default USBTX/USBRX pins
// These are typically connected to the Virtual COM Port via the debugger (e.g., ST-Link)
// Setting baud rate here.
static BufferedSerial serial_port(USBTX, USBRX, DATA_BAUD_RATE);

// DigitalOut for status indicator
static DigitalOut status_led(STATUS_LED);

// Character buffer for assembling incoming lines
char rx_buffer[RX_BUFFER_SIZE];
int rx_index = 0; // Current position in the buffer

int main(void) {
    // ... (setup) ...
    printf("Waiting for data...\n\n");
    long long mainLoopCounter = 0; // Add counter

    while (true) {
        mainLoopCounter++;
        // Optional: Blink another LED or print '.' periodically to confirm main loop is alive
        // if (mainLoopCounter % 200 == 0) { printf("."); fflush(stdout); }

        if (serial_port.readable()) {
            uint32_t num_bytes_read = serial_port.read(rx_buffer + rx_index, RX_BUFFER_SIZE - 1 - rx_index);

            if (num_bytes_read > 0) {
                rx_index += num_bytes_read;
                rx_buffer[rx_index] = '\0';
                // DEBUG: Print raw received content immediately
                // printf("Raw Read (%lu bytes): \"%s\"\n", num_bytes_read, rx_buffer);

                char* newline_ptr = strchr(rx_buffer, '\n');
                while (newline_ptr != nullptr) { // Process all complete lines in the buffer
                    *newline_ptr = '\0';
                    // DEBUG: Print the line we are about to parse
                    // printf("Processing line: \"%s\"\n", rx_buffer);

                    double pitch = 0.0, roll = 0.0, yaw = 0.0;
                    int items_parsed = sscanf(rx_buffer, "Pitch:%lf,Roll:%lf,Yaw:%lf", &pitch, &roll, &yaw);

                    if (items_parsed == 3) {
                        printf("Received -> Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", pitch, roll, yaw);
                        status_led = !status_led;
                    } else {
                        // Make error more prominent
                        printf("\n*** PARSING ERROR ***\n");
                        printf("Malformed line content: \"%s\"\n", rx_buffer);
                        printf("Items parsed: %d (expected 3)\n", items_parsed);
                        printf("*** END PARSING ERROR ***\n");
                        // Consider clearing the whole buffer on error?
                        // rx_index = 0; rx_buffer[0] = '\0'; break; // Exit inner while to prevent processing garbage
                    }

                    int remaining_chars = rx_index - ( (newline_ptr - rx_buffer) + 1 );
                    if (remaining_chars > 0) {
                        memmove(rx_buffer, newline_ptr + 1, remaining_chars);
                        rx_index = remaining_chars;
                        rx_buffer[rx_index] = '\0';
                         // Check for another newline immediately in the remaining data
                        newline_ptr = strchr(rx_buffer, '\n');
                    } else {
                        rx_index = 0;
                        rx_buffer[0] = '\0';
                        newline_ptr = nullptr; // No more data in buffer
                    }
                } // End while(newline_ptr != nullptr)

                if (rx_index >= RX_BUFFER_SIZE - 1) {
                     printf("\n*** BUFFER FULL without newline! ***\n");
                     printf("Buffer content: \"%s\"\n", rx_buffer);
                     printf("Clearing buffer.\n");
                     printf("*** END BUFFER FULL ***\n");
                    rx_index = 0;
                    rx_buffer[0] = '\0';
                }
            } // End if (num_bytes_read > 0)
        } // End if (serial_port.readable())

        ThisThread::sleep_for(5ms);
    } // end while(true)
}