#ifndef USER_H
#define USER_H

#include "stm32f4xx_hal.h"

void user_init();
void user_loop();

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
// void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

#endif // ifndef USER_H