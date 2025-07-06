#ifndef COMMON_H
#define COMMON_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define DEBUG_PRINT(huart, fmt, ...) { \
    char print[128]; \
    snprintf(print, sizeof(print), fmt, __VA_ARGS__); \
    HAL_UART_Transmit(&huart, (uint8_t *)print, strlen(print), HAL_MAX_DELAY); \
}

#define DEBUG_PRINT_SIMPLE(huart, str) { \
    char print[128]; \
    snprintf(print, sizeof(print), str); \
    HAL_UART_Transmit(&huart, (uint8_t *)print, strlen(print), HAL_MAX_DELAY); \
}

#define DEBUG_PRINT_IT(huart, fmt, ...) { \
    char print[128]; \
    snprintf(print, sizeof(print), fmt, __VA_ARGS__); \
    HAL_UART_Transmit_IT(&huart, (uint8_t *)print, strlen(print)); \
}

int8_t plusmod(int8_t a, int8_t b);

// I think integer wrapping does this for me but uhhhhhhh
uint32_t timerDiffWrapped(uint32_t a, uint32_t b);

// quick utility function
// checks whether delay has passed since prevTimer
// if so, prevTimer is reset
bool compareTimer(uint32_t * prevTimer, uint32_t delay);

// checks whether condition is met and delay has passed since prevTimer
// if both are true, prevTimer is reset
bool compareTimerCond(uint32_t * prevTimer, uint32_t delay, bool condition);

#endif // ifndef COMMON_H