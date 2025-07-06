#ifndef MADGWICK_H
#define MADGWICK_H

#include "stm32f4xx_hal.h"
#include "math_utils.h"
#include "imu_drv.h"

typedef struct {
    IMU* imu;
    uint8_t readStatusIMU; // 0 = idle, 1 = reading gyro, 2 = reading accel

    uint8_t imu_accel_data[6];
    uint8_t imu_gyro_data[6];

    GPIO_TypeDef* indicator_GPIO_port;
    uint16_t indicator_GPIO_pin;

    bool i2cEnable;

    Vec3 gyro_drift; // drift of gyro in rad/s
    Vec3 accel_drift; // drift of accel in g's

    double madgwick_beta;

    Vec4 dir;
} MFilt;

void MFilt_Init(MFilt * mfilt);

void MFilt_Update(MFilt * mfilt, float delta_t_s);

void MFilt_I2CCallback(MFilt * mfilt, I2C_HandleTypeDef *hi2c);

void MFilt_Reset(MFilt * mfilt);

void MFilt_StartRead(MFilt * mfilt);

#endif