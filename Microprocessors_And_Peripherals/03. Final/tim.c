/**
 * A custom timer using NUCLEO's TIM2 Reg
 */

#include "tim.h"

#include "stm32f4xx.h"

static void (*timerCallback)(void) = 0;

void timerInitialize(uint32_t duration) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // Enable TIM2 clock

    // Calculate the prescaler and ARR values
    uint32_t prescaler = (SystemCoreClock / 1000) - 1;
    uint32_t arr = duration - 1;

    TIM2->PSC = prescaler;  // Set prescaler value
    TIM2->ARR = arr;        // Set auto-reload value based on the input milliseconds

    TIM2->DIER |= TIM_DIER_UIE;  // Enable update interrupt (UIE)
    // TIM2->CR1 |= TIM_CR1_CEN;    // Enable the counter
    NVIC_EnableIRQ(TIM2_IRQn);       // Enable TIM2 interrupt in NVIC
    NVIC_SetPriority(TIM2_IRQn, 1);  // Set priority (optional)
}

void timerEnable() {
    TIM2->CR1 |= TIM_CR1_CEN;  // Enable the counter
}

void timerDisable() {
    TIM2->CR1 &= ~TIM_CR1_CEN;  // Disable the counter
}

void timerChange(uint32_t duration) { TIM2->ARR = duration - 1; }

void TIM2_IRQHandler(void) {
    // Clear TIM2 update interrupt flag.
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        timerCallback();
    }
}

void timerSetCallback(void (*callback)(void)) { timerCallback = callback; }