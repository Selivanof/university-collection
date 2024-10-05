#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "queue.h"
#include "stm32f4xx.h"
#include "uart.h"

#define BUFF_SIZE 128

extern int digital_root(int);
extern int sum_of_natural_numbers(int);
extern int hashfunc(const char* input);

Queue rx_queue;  // Queue for storing received characters

// Interrupt Service Routine for UART receive
void uart_rx_isr(uint8_t rx) {
    // Check if the received character is a printable ASCII character
    if (rx >= 0x0 && rx <= 0x7F) {
        // Store the received character
        queue_enqueue(&rx_queue, rx);
    }
}

const char* get_uart_input(char* buff) {
    // Variables to help with UART read
    uint8_t rx_char = 0;
    uint32_t buff_index;

    // Initialize the receive queue and UART
    queue_init(&rx_queue, 128);
    uart_init(115200);
    uart_set_rx_callback(uart_rx_isr);
    uart_enable();
    // Enable Interrupts
    __enable_irq();

    uart_print("\r\n");

    // Prompt the user to enter their string
    uart_print("Enter a string to hash:");
    buff_index = 0;  // Reset buffer index

    do {
        // Wait until a character is received in the queue
        while (!queue_dequeue(&rx_queue, &rx_char)) __WFI();  // Wait for Interrupt

        if (rx_char == 0x7F) {  // Handle backspace character
            if (buff_index > 0) {
                buff_index--;      // Move buffer index back
                uart_tx(rx_char);  // Send backspace character to erase on terminal
            }
        } else {
            // Store and echo the received character back
            buff[buff_index++] = (char)rx_char;  // Store character in buffer
            uart_tx(rx_char);                    // Echo character back to terminal
        }
    } while (rx_char != '\r' && buff_index < BUFF_SIZE);  // Continue until Enter key or buffer full

    // Replace the last character with null terminator to make it a valid C string
    buff[buff_index - 1] = '\0';
    uart_print("\r\n");  // Print newline

    // Check if buffer overflow occurred
    if (buff_index > BUFF_SIZE) {
        uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
    }
    return buff;
}

// Prints a result to both uart and debug output
void print_result(const char* label, int result) {
    char output[BUFF_SIZE * 2];
    sprintf(output, "%s: %d\r\n", label, result);
    uart_print(output);
    printf("%s", output);
}

int main() {
    char input[BUFF_SIZE];
    // Get user input
    get_uart_input(input);
    printf("Received input %s \n", input);
    // Hashing
    int hash_result = hashfunc(input);
    print_result("Hash", hash_result);
    // Digital Root
    int digital_root_result = digital_root(hash_result);
    print_result("Digital Root", digital_root_result);
    // Sum of Natural Numbers
    int sum_of_int_result = sum_of_natural_numbers(digital_root_result);
    print_result("Sum of Natural Numbers", sum_of_int_result);
}
