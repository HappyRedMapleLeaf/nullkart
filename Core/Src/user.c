#include "user.h"
#include "main.h"
#include "common.h"
#include <stdbool.h>
#include <math.h>

#include "math_utils.h"
#include "imu_drv.h"
#include "pid.h"

#include "dcmotor_drv.h"

#include "ble.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;

extern uint16_t service_hndl, characteristic_hndl;

float left_power = 0.0;
float right_power = 0.0;

extern uint16_t service_hndl, characteristic_hndl;

extern bool paired;

// L PWM PB4 3/1; DIR PB5, PA10; ENC PC2,  PC3
// R PWM PC7 3/2; DIR PC0, PA9;  ENC PA15, PB7

DCMotor left_wheel = {
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_1,
    .fwd_GPIO_port = L_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = L_DIR_1_Pin,
    .rev_GPIO_port = L_DIR_2_GPIO_Port,
    .rev_GPIO_pin = L_DIR_2_Pin,
    .enc_a_GPIO_port = L_ENC_1_GPIO_Port,
    .enc_a_GPIO_pin = L_ENC_1_Pin,
    .enc_b_GPIO_port = L_ENC_2_GPIO_Port,
    .enc_b_GPIO_pin = L_ENC_2_Pin,
    .htim_enc = &htim2,
};

DCMotor right_wheel = {
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_2,
    .fwd_GPIO_port = R_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = R_DIR_1_Pin,
    .rev_GPIO_port = R_DIR_2_GPIO_Port,
    .rev_GPIO_pin = R_DIR_2_Pin,
    .enc_a_GPIO_port = R_ENC_1_GPIO_Port,
    .enc_a_GPIO_pin = R_ENC_1_Pin,
    .enc_b_GPIO_port = R_ENC_2_GPIO_Port,
    .enc_b_GPIO_pin = R_ENC_2_Pin,
    .htim_enc = &htim2,
};

void user_init() {
    HAL_TIM_Base_Start(&htim5);
    DCMotor_Init(&left_wheel);
    DCMotor_Init(&right_wheel);
    MX_BlueNRG_2_Init();
}

void user_loop() {
    MX_BlueNRG_2_Process();
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

    // update values in BLE characteristic
    if (htim == &htim2 && paired) {
        uint8_t values[16];
        // copy volatile values
        uint64_t left_ticks = left_wheel.ticks;
        uint64_t right_ticks = right_wheel.ticks;
        memcpy(&(values[0]),    &(left_ticks),  sizeof(uint64_t));
        memcpy(&(values[8]), &(right_ticks), sizeof(uint64_t));

        aci_gatt_update_char_value(service_hndl, characteristic_hndl, 0,
                                16, values);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	DCMotor_GPIOCallback(&left_wheel, GPIO_Pin);
	DCMotor_GPIOCallback(&right_wheel, GPIO_Pin);
}
