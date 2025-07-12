#include "user.h"
#include "main.h"
#include "common.h"
#include <stdbool.h>
#include <math.h>

#include "math_utils.h"
#include "imu_drv.h"
#include "pid.h"

#include "dcmotor_drv.h"

#include "app_bluenrg_2.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;

float left_power = 0.0;
float right_power = 0.0;

extern uint16_t service_hndl, characteristic_hndl;

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
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_2,
    .fwd_GPIO_port = R_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = R_DIR_1_Pin,
    .rev_GPIO_port = R_DIR_2_GPIO_Port,
    .rev_GPIO_pin = R_DIR_2_Pin,
};

void user_init() {
    HAL_TIM_Base_Start(&htim5);
    DCMotor_Start(&left_wheel);
    DCMotor_Start(&right_wheel);
    MX_BlueNRG_2_Init();
}

float power = 0.0;
bool up = false;

void user_loop() {
    MX_BlueNRG_2_Process();

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
    // DCMotor_SetPower(&left_wheel, 0.1);
    // DCMotor_SetPower(&right_wheel, 0.1);
}

// attribute changes its value
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attribute_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[]) {
    if (Attribute_Handle == characteristic_hndl + 1) {
        // received data from client
        if (Attr_Data_Length >= 8) {  // 8 bytes = 2 floats × 4 bytes each            
            // Copy bytes to float variables (handles alignment issues)
            memcpy(&left_power, &Attr_Data[0], sizeof(float));   // First 4 bytes
            memcpy(&right_power, &Attr_Data[4], sizeof(float));  // Next 4 bytes

            DCMotor_SetPower(&left_wheel, left_power);
            DCMotor_SetPower(&right_wheel, right_power);
        }
    }
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
