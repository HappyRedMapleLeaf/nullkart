#ifndef GLOBALS_H
#define GLOBALS_H

#include "stm32f4xx_hal.h"
#include "math_utils.h"
#include "pid.h"

#include <stdbool.h>

volatile bool txReady = true;
volatile uint8_t txCount = 0;

float target_rate = 0.0;

uint32_t prev_time_tx = 0;
uint32_t prev_time_pid = 0;

PID_Controller flywheelMotor = {
    .kp = 0.002,
    .ki = 0.000004,
    .kd = 0.001,
    .integral_error = 0.0,
    .prev_error = 0.0
};

uint8_t rxData[4] = {0, 0, 0, 0};
bool rxed = true;
uint32_t prev_time_UART_rx = 0;

PID_Controller angleController = {
    .kp = 3.5,
    .ki = 0.0,
    .kd = 1.5,
    .integral_error = 0.0,
    .prev_error = 0.0
};

// IMU
uint32_t prev_time_imu_rx = 0;    // main loop timing

Vec4 dir = {0.0, 0.0, 0.0, 1.0};
float theta = 0.0;
const double madgwick_beta = 0.08; // as magic as magic numbers get
Vec3 gyro_drift;
Vec3 accel_drift;

bool i2cEnable = false;
uint8_t imu_gyro_data[6];
uint8_t imu_accel_data[6];
uint8_t readStatusIMU = 0;  // 0: ready, 1: reading gyro, 2: reading accel

#endif // GLOBALS_H
