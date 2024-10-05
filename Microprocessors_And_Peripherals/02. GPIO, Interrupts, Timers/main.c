/*
  Author: selivanof(George Selivanof), chatziiola (Lamprinos Chatziioannou)
  Date: 2024-05-10
  Description: Final version of what is asked of us for project 2 of mp-labs
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpio.h"
#include "platform.h"
#include "queue.h"
#include "stdbool.h"
#include "stm32f4xx.h"
#include "timer.h"
#include "uart.h"

#define LEDPIN PA_5
#define LEDSEMIPERIOD 500000
#define BUFF_SIZE 6  // Conscious decision since AEM consists of only 5 digits
#define QUEUE_SIZE 20

static Queue rx_queue;
static Queue msg_queue;
static int switchPresses;

// switchPressed_isr (status)
// - once this is called the following actions occur:
// 1. The timer is disabled (thus no more LED toggling states)
// 2. The LED changes state *once*
// 3. Both the keypress and the led change are added to message queue
//    so that they are eventually printed by uart.
void switchPressed_isr(int status) {
    timer_disable();
    gpio_toggle(LEDPIN);
    // The shifting trick is but a simple hack: knowing that
    // it is pin LEDPIN that holds the status of the pin,
    // shifting it allows us to easily get its value.
    // - in a more complex environment a mask should also be used.
    queue_enqueue(&msg_queue, gpio_get(LEDPIN) >> LEDPIN);
    queue_enqueue(&msg_queue, 2);
}

// timer_callback_isr
// Whenever there is a timer interrupt:
// 1. Toggle LEDPIN state
// 2. Add the change to msg_queue, so that it gets printed
void timer_callback_isr(void) {
    // Avoiding to overpopulate the isr
    gpio_toggle(LEDPIN);
    queue_enqueue(&msg_queue, gpio_get(LEDPIN) >> LEDPIN);
}

// uart_rx_isr
// Whenever there is input: Simply add it to rx_queue.
// checkUart handles the actual message
void uart_rx_isr(uint8_t rx) {
    // Check if the received character is a printable ASCII character
    if (rx >= 0x0 && rx <= 0x7F) {
        // Store the received character
        queue_enqueue(&rx_queue, rx);
    }
}

// checkMessg
// See whether there exists a message to be printed.
// 1. Check for messages in msg_queue
// If found:
// a. if code 0: it means the LED went off
// b. if code 1: it means the LED went on
// c. if code 2: it means the button was pressed, shows number of times.
bool checkMessg() {
    // This is set to -1 to make sure that the values get properly
    // assigned later on
    int lastState = -1;
    char buffer[50];
    if (queue_dequeue(&msg_queue, &lastState)) {
        switch (lastState) {
            case 0:
                sprintf(buffer, "Led is off.\r\n");
                break;
            case 1:
                sprintf(buffer, "Led is on.\r\n");
                break;
            case 2:
                sprintf(buffer, "Keypresses pressed %d .\r\n", ++switchPresses);
                break;
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

// checkAEM
// Check buffer (should be complete), and test
// if aem is digits only, otherwise it does not get parsed
// if odd: enable the timer (led starts blinking)
// if even: disable timer (led remains steady)
void checkAEM(char *buff) {
    if (isValidAEM(buff)) {
        int aem = atoi(buff);
        if (aem % 2)  // odd
            timer_enable();
        else  // even
            timer_disable();
    }
}

// checkUART
// - buff_index: is the current index (thus the current length)
// - buff: the input string so far.

// checkUART handles the user input, after the initial characters have been
// handled by uart_ISR: heavily utilizing parts of the given uart function, but
// bundling them together in a way they could be elegantly inserted in main: It
// checks the rx_queue with newer input to update the buff, and if such is found
// then:
// - if backspace: special case: update screen
// - else if enter: update buff (end string), checkAEM, and reset
// buff,buff_index
// - else (if normal char): append it to buff
bool checkUart(uint32_t *buff_index, char *buff) {
    uint8_t rx_char = 0;
    if (queue_dequeue(&rx_queue, &rx_char)) {
        // Handle backspace character
        if (rx_char == 0x08) {
            if (*buff_index > 0) {
                (*buff_index)--;
                uart_tx(rx_char);
            }
        } else if (rx_char == '\r') {
            uart_print("\r\n");
            checkAEM(buff);
            *buff_index = 0;
            memset(buff, '\0', BUFF_SIZE);
            uart_print("\r\nEnter your AEM:");
        } else if (*buff_index >= BUFF_SIZE) {
            uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
        } else {
            // Store and echo the received character back
            buff[(*buff_index)++] = (char)rx_char;  // Store character in buffer
            uart_tx(rx_char);                       // Echo character back to terminal
        }
        return true;
    }
    return false;
}

int main() {
    //**************************************************
    // Variable initialization
    //**************************************************
    char buff[BUFF_SIZE];
    switchPresses = 0;
    uint32_t buff_index = 0;

    //**************************************************
    // Queues
    // - msg_queue, contains any messages to be printed
    // - rx_queue, is used to handle uart input
    //**************************************************
    queue_init(&msg_queue, QUEUE_SIZE);
    queue_init(&rx_queue, 128);

    //**************************************************
    // UART initialization
    //**************************************************
    uart_init(115200);
    uart_set_rx_callback(uart_rx_isr);  // Set the UART receive callback function
    uart_enable();                      // Enable UART module

    //**************************************************
    // P_SW Button initialization
    //**************************************************
    gpio_set_mode(P_SW, Input);
    gpio_set_callback(P_SW, switchPressed_isr);
    gpio_set_trigger(P_SW, Rising);
    gpio_set_mode(LEDPIN, Output);

    //**************************************************
    // Timer Initialization
    //**************************************************
    timer_init(LEDSEMIPERIOD);
    timer_set_callback(timer_callback_isr);
    // Timer is initially disabled because that is the starting state.
    // It gets enabled when needed by call of the appropriate functions
    timer_disable();

    // Enable interrupts
    __enable_irq();

    uart_print("\r\nEnter your AEM:");

    // While the application runs
    while (1) {
        // There is nothing that needs to be actively done outside of
        // - checkUart() -> handling input,
        //   seeing whether the latest uart interrupt has any actual meaning,
        // - checkMessg() -> handling output,
        //   printing any message there may be in msg_queu
        //
        // NOTE: __WFI is fully compatible with this semi-polling
        // approach since whatever change, button press/uart input
        // is sure to cause an interrupt
        if (!(checkUart(&buff_index, buff) || checkMessg())) __WFI();
    }

    __disable_irq();
}
