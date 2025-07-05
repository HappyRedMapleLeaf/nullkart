#ifndef DCMOTOR_DRV_H
#define DCMOTOR_DRV_H

#include "stm32f4xx_hal.h"

#define RATE_AVG_BUF_SIZE 5

typedef struct {
    // data used for PWM setting
    TIM_HandleTypeDef *htim_pwm;
    uint32_t pwm_channel;

    // data used for direction control
    GPIO_TypeDef* fwd_GPIO_port;
    uint16_t fwd_GPIO_pin;
    GPIO_TypeDef* rev_GPIO_port;
    uint16_t rev_GPIO_pin;

    // // data used for encoder
    // GPIO_TypeDef* ENC1_A_GPIO_Port;
    // uint16_t ENC1_A_Pin;
    // GPIO_TypeDef* ENC1_B_GPIO_Port;
    // uint16_t ENC1_B_Pin;
    // TIM_HandleTypeDef *htim_enc;

    // // TODO: init to 0
    // volatile float rateAvgBuffer[RATE_AVG_BUF_SIZE];
    // volatile uint16_t rateAvgIdx;
    // volatile float rateAvgSum;
    // volatile float rate_filtered;

    // int64_t ticks;
    // int64_t prev_ticks;
} DCMotor;

void DCMotor_Start(DCMotor *motor);

void DCMotor_TIMCallback(DCMotor *motor, TIM_HandleTypeDef *htim, float dt_s);

void DCMotor_GPIOCallback(DCMotor *motor, uint16_t GPIO_Pin);

void DCMotor_SetPower(DCMotor *motor, float power);

#endif // DCMOTOR_DRV_H