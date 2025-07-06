#include "user.h"
#include "main.h"
#include "common.h"
#include <stdbool.h>
#include <math.h>

#include "math_utils.h"
#include "imu_drv.h"
#include "pid.h"

#include "dcmotor_drv.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;

// L PWM PB4 3/1; DIR PB5, PA10; ENC PC2, PC3
// R PWM PC7 3/2; DIR PC0, PA9; ENC PH0, PH1

DCMotor left_wheel = {
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_1,
    .fwd_GPIO_port = L_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = L_DIR_1_Pin,
    .rev_GPIO_port = L_DIR_2_GPIO_Port,
    .rev_GPIO_pin = L_DIR_2_Pin,
};

DCMotor right_wheel = {
    .htim_pwm = &htim2,
    .pwm_channel = TIM_CHANNEL_2,
    .fwd_GPIO_port = R_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = R_DIR_1_Pin,
    .rev_GPIO_port = R_DIR_2_GPIO_Port,
    .rev_GPIO_pin = R_DIR_2_Pin,
};

void user_init() {
    HAL_TIM_Base_Start(&htim5);
}

float power = 0.0;
bool up = false;

void user_loop() {    
    // if (power < 0.0) {
    //     power = 0.0;
    //     up = true;
    // } else if (power > 1.0) {
    //     power = 1.0;
    //     up = false;
    // }
    // if (up) {
    //     power += 0.01;
    // } else {
    //     power -= 0.01;
    // }
    // DCMotor_SetPower(&left_wheel, power);
    // DCMotor_SetPower(&right_wheel, power);
    DCMotor_SetPower(&left_wheel, 1);
    DCMotor_SetPower(&right_wheel, 1);

    HAL_Delay(10); // crucially important
    // if the loop is too fast some weird race condition thing happens and the uart transmission messes up
    // will investigate further later (maybe)
}

// currently every 10ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
	DCMotor_TIMCallback(&left_wheel, htim, 0.01F);
	DCMotor_TIMCallback(&right_wheel, htim, 0.01F);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	DCMotor_GPIOCallback(&left_wheel, GPIO_Pin);
	DCMotor_GPIOCallback(&right_wheel, GPIO_Pin);
}
