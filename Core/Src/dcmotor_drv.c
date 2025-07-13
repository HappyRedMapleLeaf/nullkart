#include "dcmotor_drv.h"

#include <math.h>

void DCMotor_Init(DCMotor *motor) {
    // Initialize encoder values
    motor->rateAvgIdx = 0;
    motor->rateAvgSum = 0.0f;
    motor->rate_filtered = 0.0f;
    motor->ticks = 0;
    motor->prev_ticks = 0;

    // Initialize rate average buffer
    for (int i = 0; i < RATE_AVG_BUF_SIZE; i++) {
        motor->rateAvgBuffer[i] = 0.0f;
    }

    // Start the encoder timer
    HAL_TIM_Base_Start_IT(motor->htim_enc);
    HAL_TIM_PWM_Start(motor->htim_pwm, motor->pwm_channel);
}

void DCMotor_TIMCallback(DCMotor *motor, TIM_HandleTypeDef *htim, float dt_s) {
    if (htim == motor->htim_enc) {
        // calculate raw derivative of ticks
        float rate = (motor->ticks - motor->prev_ticks) / dt_s;
        motor->prev_ticks = motor->ticks;

        // Remove the oldest sample from the sum
        motor->rateAvgSum -= motor->rateAvgBuffer[motor->rateAvgIdx];
        motor->rateAvgBuffer[motor->rateAvgIdx] = rate;
        motor->rateAvgSum += rate;
        motor->rateAvgIdx = (motor->rateAvgIdx + 1) % RATE_AVG_BUF_SIZE;
        motor->rate_filtered = motor->rateAvgSum / RATE_AVG_BUF_SIZE;
    }
}

void DCMotor_GPIOCallback(DCMotor *motor, uint16_t GPIO_Pin) {
    if (GPIO_Pin == motor->enc_a_GPIO_pin) {
        if (HAL_GPIO_ReadPin(motor->enc_a_GPIO_port, motor->enc_a_GPIO_pin) == HAL_GPIO_ReadPin(motor->enc_b_GPIO_port, motor->enc_b_GPIO_pin)) {
            // going backwards if falling edge on channel B low
            // or rising edge on channel B high
            motor->ticks--;
        } else {
            // going forwards if rising edge on channel B low
            // or falling edge on channel B high
            motor->ticks++;
        }
    } else if (GPIO_Pin == motor->enc_b_GPIO_pin) {
        if (HAL_GPIO_ReadPin(motor->enc_a_GPIO_port, motor->enc_a_GPIO_pin) == HAL_GPIO_ReadPin(motor->enc_b_GPIO_port, motor->enc_b_GPIO_pin)) {
            // going forwards if rising edge on channel A low
            // or falling edge on channel A high
            motor->ticks++;
        } else {
            // going backwards if falling edge on channel A low
            // or rising edge on channel A high
            motor->ticks--;
        }
    }
}

void DCMotor_SetPower(DCMotor *motor, float power) {
    if (power > 1.0) {
        power = 1.0;
    }
    if (power < -1.0) {
        power = -1.0;
    }
    uint16_t pwm = (uint16_t) (fabs(power)*4200.0);
    __HAL_TIM_SET_COMPARE(motor->htim_pwm, motor->pwm_channel, pwm);
    if (power > 0) {
        HAL_GPIO_WritePin(motor->fwd_GPIO_port, motor->fwd_GPIO_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->rev_GPIO_port, motor->rev_GPIO_pin, GPIO_PIN_RESET);
    }
    // coast if power == 0
    if (power < 0) {
        HAL_GPIO_WritePin(motor->rev_GPIO_port, motor->rev_GPIO_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->fwd_GPIO_port, motor->fwd_GPIO_pin, GPIO_PIN_RESET);
    }
}
