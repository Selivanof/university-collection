#include "stdint.h"

void timerEnable();
void timerDisable();
void timerChange(uint32_t duration);
void timerSetCallback(void (*callback)());
void timerInitialize(uint32_t duration);