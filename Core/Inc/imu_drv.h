#ifndef IMU_DRV_H
#define IMU_DRV_H

#include "math_utils.h"

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define G_TO_MM_S_2 9806.65

typedef struct {
    I2C_HandleTypeDef *i2c;
} IMU;

void IMU_Init(IMU * imu);

// expect 0xE1
uint8_t IMU_WhoAmI(IMU * imu);

void IMU_WhoAmIIT(IMU * imu, uint8_t * res);

// call IMU_Init() after
void IMU_Reset(IMU * imu);

// return unit: g's
float IMU_Read_Accel(IMU * imu, uint8_t axis);

// return unit: deg/s
float IMU_Read_Gyro(IMU * imu, uint8_t axis);

// return unit: celsius
float IMU_Read_Temp(IMU * imu);

// 0b00 = +/- 4g
// 0b01 = +/- 8g
// 0b10 = +/- 16g
// 0b11 = +/- 30g
void IMU_Set_Accel_Range(IMU * imu, uint8_t accel_fs_new);

// 00 = +/- 500 dps
// 01 = +/- 1000 dps
// 10 = +/- 2000 dps
// 11 = +/- 4000 dps
void IMU_Set_Gyro_Range(IMU * imu, uint8_t gyro_fs_new);

// // units: mm/s^2
// Vec3 IMU_Read_Accel_Vec3();

// // units: rad/s
// Vec3 IMU_Read_Gyro_Vec3();

void IMU_StartReadGyroIT(IMU * imu, uint8_t * data);
Vec3 IMU_ConvertGyro(uint8_t * data);
void IMU_StartReadAccelIT(IMU * imu, uint8_t * data);
Vec3 IMU_ConvertAccel(uint8_t * data);

void IMU_StartReadGyroAxisIT(IMU * imu, uint8_t * data, uint8_t axis);
float IMU_ConvertGyroAxis(uint8_t * data);

Vec3 IMU_Read_Gyro_Vec3(IMU * imu);
Vec3 IMU_Read_Accel_Vec3(IMU * imu);

#endif /* IMU_DRV_H */
