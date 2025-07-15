#include "user.h"

/* STANDARD INCLUDES */
#include <stdbool.h>
#include <math.h>

/* USER INCLUDES */
#include "ble.h"
#include "dcmotor_drv.h"
#include "imu_drv.h"
#include "main.h"
#include "math_utils.h"
#include "pid.h"
#include "two_wheel_tracker.h"

/* EXTERN VARIABLES */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;
extern uint16_t service_hndl, txchar_hndl, rxchar_hndl;
extern bool paired;

/* GLOBAL VARIABLES */
float left_power = 0.0;
float right_power = 0.0;
uint8_t send_values[16];
uint32_t prev_updatechar_ticks = 0;
uint32_t prev_localize_ticks = 0;
uint8_t irq_ignore_count = 4;

/* GLOBAL OBJECTS */

// L PWM PB4 3/1; DIR PB5, PA10; ENC PC2,  PC3
// R PWM PC7 3/2; DIR PC0, PA9;  ENC PA15, PB7

DCMotor left_wheel = {
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_1,
    .fwd_GPIO_port = L_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = L_DIR_1_Pin,
    .rev_GPIO_port = L_DIR_2_GPIO_Port,
    .rev_GPIO_pin = L_DIR_2_Pin,
    .enc_a_GPIO_port = L_ENC_2_GPIO_Port,
    .enc_a_GPIO_pin = L_ENC_2_Pin,
    .enc_b_GPIO_port = L_ENC_1_GPIO_Port,
    .enc_b_GPIO_pin = L_ENC_1_Pin,
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

TwoWheelTracker tracker = {
    .left_wheel = &left_wheel,
    .right_wheel = &right_wheel,
    .ticks_per_rev = 28,
    .wheel_radius = 0.021, // 21mm (approx. update later)
    .track_width = 0.24    // 240mm (approx. update later)
};

/* PRIVATE FUNCTION DECLARATIONS */

/* FUNCTION DEFINITINS */

void user_init() {
    HAL_TIM_Base_Start(&htim5); // general timer
    DCMotor_Init(&left_wheel);
    DCMotor_Init(&right_wheel);
    TwoWheelTracker_Init(&tracker);
    MX_BlueNRG_2_Init();
}

// quick utility function
// checks whether delay has passed since prevTimer
// if so, prevTimer is reset
// to be replaced with some sort of timer module
bool compareTimer(uint32_t * prevTimer, uint32_t delay) {
    bool delayReached = TIM5->CNT - *prevTimer > delay;
    if (delayReached) {
        *prevTimer = TIM5->CNT;
    }
    return delayReached;
}

void user_loop() {
    MX_BlueNRG_2_Process();

    // 250ms loop
    if (compareTimer(&prev_updatechar_ticks, 64000*250)) {
        if (irq_ignore_count > 0) {
            irq_ignore_count--;
        }
        // update values in BLE characteristic (ie transmit data)
        if (paired) {
            // copy volatile values
            // uint64_t left_ticks = left_wheel.ticks;
            // uint64_t right_ticks = right_wheel.ticks;
            // memcpy(&(send_values[0]), &(left_ticks),  sizeof(uint64_t));
            // memcpy(&(send_values[8]), &(right_ticks), sizeof(uint64_t));

            // float left_rate = left_wheel.rate_filtered;
            // float right_rate = right_wheel.rate_filtered;
            // memcpy(&(send_values[0]), &(left_rate),  sizeof(float));
            // memcpy(&(send_values[4]), &(right_rate), sizeof(float));

            float x = tracker.position.x;
            float y = tracker.position.y;
            float heading = tracker.heading;
            memcpy(&(send_values[0]), &(x), sizeof(float));
            memcpy(&(send_values[4]), &(y), sizeof(float));
            memcpy(&(send_values[8]), &(heading), sizeof(float));

            aci_gatt_update_char_value(service_hndl, txchar_hndl, 0,
                                       16, send_values);
        }
    }

    // 100ms loop
    if (compareTimer(&prev_updatechar_ticks, 64000*100)) {
        if (paired && irq_ignore_count == 0) {
            TwoWheelTracker_Update(&tracker, 0.1F);
        }
    }
}

/* CALLBACKS */

// handle when attribute changes its value (ie. we receive data)
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attribute_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[]) {
    if (Attribute_Handle == rxchar_hndl + 1) {
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
    if (irq_ignore_count == 0) {
        DCMotor_TIMCallback(&left_wheel, htim, 0.01F);
        DCMotor_TIMCallback(&right_wheel, htim, 0.01F);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (irq_ignore_count == 0) {
        DCMotor_GPIOCallback(&left_wheel, GPIO_Pin);
        DCMotor_GPIOCallback(&right_wheel, GPIO_Pin);
    }
}
