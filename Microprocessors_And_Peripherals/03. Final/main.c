#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dht.h"
#include "float_queue.h"
#include "gpio.h"
#include "platform.h"
#include "queue.h"
#include "stdbool.h"
#include "stm32f4xx.h"
#include "tim.h"
#include "timer.h"
#include "uart.h"

#define LEDPIN PA_4
#define LEDSEMIPERIOD 5000000
#define BUFF_SIZE 6  // Conscious decision since AEM consists of only 5 digits
#define QUEUE_SIZE 20

static Queue rx_queue;
static FloatQueue msg_queue;
static int switchPresses = 0;
static bool showHumidity = false;
static uint8_t sensorRate = 2;
static char buff[BUFF_SIZE];

/**
 * The ISR for the touch sensor. Based on the requirements:
 * 1st Press: Set the sensor rate to the sum of the last 2 digits of AEM (unless that sum is 2, then set to 4)
 * Even Press Count: Disable showing humidity info
 * Odd Press Count: Enable showing humidity info and set the sensor rate to 3
 **/
static void TouchSensorISR(int status) {
    if (switchPresses == 0) {
        int AEM = atoi(buff) % 100;
        AEM = AEM / 10 + AEM % 10;
        sensorRate = (AEM != 2) ? AEM : 4;
    } else if (switchPresses % 2) {
        sensorRate = 3;
        showHumidity = false;
    } else {
        showHumidity = true;
    }
    switchPresses++;
    timerChange(sensorRate * 1000);
}

/**
 * ISR for timer to read tempearature data.
 * Add the received data to a message queue.
 * Messages are encoded in a basic way as follows
 * Temperature data as is (0-254)
 * - Humidity data offset by 255 (255-509)
 * - Sensor rate data offset by 510 (510+)
 * - This is done to requse a simple queue for all 3 message types
 * The data are not printed by the ISR to be as quick as possible
 */
static void temperature_timer_isr(void) {
    float temperature, humidity;
    ReadDHT11(PA_1, &temperature, &humidity);
    float_queue_enqueue(&msg_queue, temperature);
    if (showHumidity) {
        float_queue_enqueue(&msg_queue, humidity + 255);
    }
    float_queue_enqueue(&msg_queue, sensorRate + 510);
}

// Simple ISR for LED blinking
static void blink_timer_isr(void) { gpio_toggle(LEDPIN); }

// UART receive ISR
static void uart_rx_isr(uint8_t rx) {
    // Check if the received character is a printable ASCII character
    if (rx >= 0x0 && rx <= 0x7F) {
        // Store the received character
        queue_enqueue(&rx_queue, rx);
    }
}

/**
 * Take a message from the queue and act on it according to its type:
 * - Temperature: Print the value and
 *      - if > 25C turn on LED
 *      - if < 20C turn off the LED
 *      - if betwwen 20C and 25C blink the LED
 * - Humidity: Print the value
 * - Sensor Rate: Print the value
 */
static bool checkMessg() {
    float msg;
    char buffer[128];
    if (float_queue_dequeue(&msg_queue, &msg)) {
        if (msg < 255) {
            if (msg > 25) {
                timer_disable();
                gpio_set(LEDPIN, 1);
            } else if (msg < 20) {
                timer_disable();
                gpio_set(LEDPIN, 0);
            } else {
                timer_enable();
            }
            sprintf(buffer, "Temperature: %f C.\r\n", (double)msg);
        } else if (msg < 510) {
            sprintf(buffer, "Humidity: %f %%.\r\n", (double)(msg - 255));
        } else {
            sprintf(buffer, "Sensor Rate: %f s .\r\n", (double)(msg - 510));
        }
        uart_print(buffer);
        return true;
    }
    return false;
}

// isValidAEM
// Checks wether or not a given buffer contains a valid AEM (a number)
static bool isValidAEM(char *buff) {
    for (int i = 0; i < BUFF_SIZE; i++) {
        if (!isdigit(buff[i])) {
            return false;
        }
    }
    return true;
}

const char *get_uart_input(char *buff) {
    // Variables to help with UART read
    uint8_t rx_char = 0;
    uint32_t buff_index;

    uart_print("\r\n");  // Print newline

    // Prompt the user to enter their string
    uart_print("Enter your AEM:");
    buff_index = 0;  // Reset buffer index

    do {
        // Wait until a character is received in the queue
        while (!queue_dequeue(&rx_queue, &rx_char)) __WFI();
        if (rx_char == 0x08) {  // Handle backspace character
            if (buff_index > 0) {
                buff_index--;      // Move buffer index back
                uart_tx(rx_char);  // Send backspace character to erase on terminal
            }
        } else {
            // Store and echo the received character back
            buff[buff_index++] = (char)rx_char;  // Store character in buffer
            uart_tx(rx_char);                    // Echo character back to terminal
        }
    } while (rx_char != '\r' && buff_index < (BUFF_SIZE - 1));  // Continue until Enter key or buffer full

    uart_print("\r\n");  // Print newline
    // Check if buffer overflow occurred
    if (buff_index > BUFF_SIZE) {
        uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
    }
    return buff;
}

int main() {
    // Create message queue
    float_queue_init(&msg_queue, QUEUE_SIZE);
    // Set LED GPIO
    gpio_set_mode(LEDPIN, Output);
    // Initialize the receive queue and UART
    queue_init(&rx_queue, 128);
    uart_init(115200);
    uart_set_rx_callback(uart_rx_isr);  // Set the UART receive callback function
    uart_enable();                      // Enable UART module
    // Enable interrupts
    __enable_irq();
    // Set the blink ISR timer to 1s
    timer_init(1000000);
    timer_set_callback(blink_timer_isr);
    timer_disable();
    // Get a valid input
    do {
        get_uart_input(buff);
    } while (!isValidAEM(buff));
    // Set touch sensor GPIO
    gpio_set_mode(PA_0, Input);
    gpio_set_callback(PA_0, TouchSensorISR);
    gpio_set_trigger(PA_0, Rising);
    // Set DHT sensor timer and ISR
    timerInitialize(2000);
    timerEnable();
    timerSetCallback(temperature_timer_isr);
    // Act on message queue
    while (1) {
        if (!checkMessg()) {
            __WFI();
        }
    }

    __disable_irq();
}
