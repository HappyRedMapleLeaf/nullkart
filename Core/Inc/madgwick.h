#ifndef MADGWICK_H
#define MADGWICK_H

#include "stm32f4xx_hal.h"
#include "math_utils.h"
#include "imu_drv.h"

typedef struct {
    IMU* imu;
    uint8_t readStatusIMU; // 0 = idle, 1 = reading gyro, 2 = reading accel

    uint8_t imu_accel_data[6];
} MFilt;

void MFilt_init(float sampleFreq);
void MFilt_update(float gx, float gy, float gz, float ax, float ay, float az);
void MFilt_get_quaternion(float *q);

void MFilt_I2CCallback(MFilt * mfilt, I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == mfilt->imu->i2c->Instance) {
        if (mfilt->readStatusIMU == 1) { // reading gyro
            IMU_StartReadAccelIT(mfilt->imu_accel_data);
            mfilt->readStatusIMU = 2;
        } else if (mfilt->readStatusIMU == 2) {
            mfilt->readStatusIMU = 0;
        }
    }
}

#endif