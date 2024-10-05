#include "dht.h"

#include "gpio.h"

static void wait_for_pin(Pin pin, int value, uint32_t timeout) {
    uint32_t start = DWT->CYCCNT;
    uint32_t duration = timeout * (SystemCoreClock / 1000000);
    while (((gpio_get(pin) >> pin) != value) && ((DWT->CYCCNT - start) < duration));
}

static void DWT_Init(void) {
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/*
 * Waits for a specific duration in microseconds.
 */
static void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t duration = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < duration);
}

/*
 * Sends the start signal to the DHT11 sensor.
 */
static uint8_t DHT11_Start(Pin pin) {
    uint8_t successful = 0;
    gpio_set_mode(pin, Output);
    gpio_set(pin, 0);
    delay_us(18000);
    gpio_set_mode(pin, PullUp);
    delay_us(40);
    if (gpio_get(pin)) {
        /*  If the sensor has not already set the pin to low wait for 80us. Under normal operation the pin should now be
         * low. */
        delay_us(80);
    }
    if (!(gpio_get(pin))) {
        /*  Since the pin was low, after 80us it should be high. */
        delay_us(80);
        if ((gpio_get(pin))) {
            successful = 1;  // Succesfully received the acknowledgement from the sensor
        }
    }
    wait_for_pin(pin, 0, 80);
    return successful;
}

/*
 * Reads a byte from the DHT11 sensor.
 */
static uint8_t DHT11_Read_Byte(Pin pin) {
    uint8_t byte;
    for (uint8_t i = 0; i < 8; i++) {
        wait_for_pin(pin, 1, 70);
        delay_us(40);
        if (!(gpio_get(pin))) {
            byte &= ~(1 << (7 - i));
        } else
            byte |= (1 << (7 - i));
        wait_for_pin(pin, 0, 70);
    }
    return byte;
}

uint8_t ReadDHT11(Pin pin, float *temperature, float *humidity) {
    uint8_t bytes[5];
    if (!DHT11_Start(pin)) {
        return 0;  // Starting DHT11 failed
    }
    for (uint8_t i = 0; i < 5; i++) {
        bytes[i] = DHT11_Read_Byte(pin);
    }
    if (bytes[4] != ((bytes[0] + bytes[1] + bytes[2] + bytes[3]) & 0xFF)) {
        return 0;  // Byte validation failed
    }
    *humidity = (float)(bytes[0] + bytes[1] / 10.f);
    *temperature = (float)(bytes[2] + bytes[3] / 10.f);
    return 1;
}
