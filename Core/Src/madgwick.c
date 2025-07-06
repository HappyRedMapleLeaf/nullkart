#include "madgwick.h"
#include <math.h>

void MFilt_Init(MFilt * mfilt) {
    mfilt->readStatusIMU = 0;
    mfilt->i2cEnable = false;
    mfilt->dir = (Vec4) {0.0, 0.0, 0.0, 1.0};
}

void MFilt_Update(MFilt * mfilt, float delta_t_s) {
    Vec3 gyro = vec3_sub(IMU_ConvertGyro(mfilt->imu_gyro_data), mfilt->gyro_drift);
    Vec3 accel = vec3_sub(IMU_ConvertAccel(mfilt->imu_accel_data), mfilt->accel_drift);
    // Vec3 accel = {100000, 0, 0}; // force no fusion
    MFilt_StartRead(mfilt);

    // https://ahrs.readthedocs.io/en/latest/filters/madgwick.html
    // note: they use wxyz but this uses xyzw so formulas will look different

    // quaternion derivative = 0.5 * (prev dir * angvel reading)
    Vec4 d_q = vec4_scale(vec4_mul(mfilt->dir, vec4_from_vec3(gyro)), 0.5);

    if (fabs(vec3_mag(accel)/G_TO_MM_S_2 - 1) < 0.02) {
        // acceleration is within 2% of 1g, use fusion
        // otherwise only use gyro reading

        Vec3 accel_n = vec3_norm(accel);

        // transposed jacobian of error function
        Mat43 jacobian_t = {
            {-2*mfilt->dir.z,  2*mfilt->dir.w, -2*mfilt->dir.x,  2*mfilt->dir.y},
            { 2*mfilt->dir.y,  2*mfilt->dir.x,  2*mfilt->dir.w,  2*mfilt->dir.z},
            {       0, -4*mfilt->dir.y, -4*mfilt->dir.z,  0      }
        };

        Vec3 error = {
            2*(mfilt->dir.y*mfilt->dir.w - mfilt->dir.x*mfilt->dir.z) - accel_n.x,
            2*(mfilt->dir.x*mfilt->dir.y + mfilt->dir.z*mfilt->dir.w) - accel_n.y,
            2*(0.5 - mfilt->dir.y*mfilt->dir.y - mfilt->dir.z*mfilt->dir.z) - accel_n.z
        };

        Vec4 gradient = mat43_mul_vec3(jacobian_t, error);

        // correct quaternion derivative by beta * _____
        d_q = vec4_sub(d_q, vec4_scale(vec4_norm(gradient), mfilt->madgwick_beta));
    }

    // double delta_t = 0.005;
    // integrate direction by incrementing by delta_t * derivative of direction
    mfilt->dir = vec4_add(mfilt->dir, vec4_scale(d_q, delta_t_s));
}

void MFilt_I2CCallback(MFilt * mfilt, I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == mfilt->imu->i2c->Instance) {
        if (mfilt->readStatusIMU == 1) { // reading gyro
            IMU_StartReadAccelIT(mfilt->imu, mfilt->imu_accel_data);
            mfilt->readStatusIMU = 2;
        } else if (mfilt->readStatusIMU == 2) {
            mfilt->readStatusIMU = 0;
        }
    }
}

void MFilt_Reset(MFilt * mfilt) {
    mfilt->i2cEnable = false;

    HAL_GPIO_WritePin(mfilt->indicator_GPIO_port, mfilt->indicator_GPIO_pin, GPIO_PIN_RESET);
    HAL_I2C_DeInit(mfilt->imu->i2c);
    HAL_Delay(100);
    HAL_I2C_Init(mfilt->imu->i2c);
    HAL_Delay(100);
    IMU_Reset(mfilt->imu);
    IMU_Init(mfilt->imu);
    HAL_Delay(100);

    uint8_t who = IMU_WhoAmI(mfilt->imu);
    // error
    if (who != 0xE1) {
        while (1) {}
    }

    // some weird shit happened here idk
    int32_t init_us = TIM5->CNT;
    uint32_t cal_cycle = 1;
    mfilt->gyro_drift = IMU_Read_Gyro_Vec3(mfilt->imu);
    Vec3 average_accel = IMU_Read_Accel_Vec3(mfilt->imu);

    // calibration for 100ms
    while (TIM5->CNT - init_us < 84000*100) {
        cal_cycle++;
        Vec3 accel = IMU_Read_Accel_Vec3(mfilt->imu);
        average_accel = vec3_add(average_accel, accel);
        Vec3 gyro = IMU_Read_Gyro_Vec3(mfilt->imu);
        mfilt->gyro_drift = vec3_add(mfilt->gyro_drift, gyro);
    }
    average_accel = vec3_scale(average_accel, 1.0 / cal_cycle);
    mfilt->gyro_drift = vec3_scale(mfilt->gyro_drift, 1.0 / cal_cycle);

    mfilt->accel_drift = vec3_sub(average_accel, (Vec3) {0, 0, G_TO_MM_S_2});

    mfilt->i2cEnable = true;
    HAL_GPIO_WritePin(mfilt->indicator_GPIO_port, mfilt->indicator_GPIO_pin, GPIO_PIN_SET);
}

void MFilt_StartRead(MFilt * mfilt) {
    IMU_StartReadGyroIT(mfilt->imu, mfilt->imu_gyro_data);
    mfilt->readStatusIMU = 1; // indicate reading gyro
}