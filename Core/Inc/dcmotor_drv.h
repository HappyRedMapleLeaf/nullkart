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

    // data used for encoder
    GPIO_TypeDef * enc_a_GPIO_port;
    uint16_t enc_a_GPIO_pin;
    GPIO_TypeDef * enc_b_GPIO_port;
    uint16_t enc_b_GPIO_pin;
    TIM_HandleTypeDef *htim_enc;

    // set to 0 in init
    volatile float rateAvgBuffer[RATE_AVG_BUF_SIZE];
    volatile uint16_t rateAvgIdx;
    volatile float rateAvgSum;
    volatile float rate_filtered; // this is the one to use. ticks per second.
    volatile int64_t ticks;
    volatile int64_t prev_ticks;
} DCMotor;

void DCMotor_Init(DCMotor *motor);

void DCMotor_TIMCallback(DCMotor *motor, TIM_HandleTypeDef *htim, float dt_s);

void DCMotor_GPIOCallback(DCMotor *motor, uint16_t GPIO_Pin);

void DCMotor_SetPower(DCMotor *motor, float power);

#endif // DCMOTOR_DRV_H
