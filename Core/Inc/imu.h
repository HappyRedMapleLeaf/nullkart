#ifndef IMU_H
#define IMU_H

#include "math_utils.h"

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define G_TO_MM_S_2 9806.65

void IMU_Init();

// expect 0xE1
uint8_t IMU_WhoAmI();
void IMU_WhoAmIIT(uint8_t * res);

// call IMU_Init() after
void IMU_Reset();

// return unit: g's
double IMU_Read_Accel(uint8_t axis);

// return unit: deg/s
double IMU_Read_Gyro(uint8_t axis);

// return unit: celsius
double IMU_Read_Temp();

// 0b00 = +/- 4g
// 0b01 = +/- 8g
// 0b10 = +/- 16g
// 0b11 = +/- 30g
void IMU_Set_Accel_Range(uint8_t accel_fs_new);

// 00 = +/- 500 dps
// 01 = +/- 1000 dps
// 10 = +/- 2000 dps
// 11 = +/- 4000 dps
void IMU_Set_Gyro_Range(uint8_t gyro_fs_new);

// // units: mm/s^2
// Vec3 IMU_Read_Accel_Vec3();

// // units: rad/s
// Vec3 IMU_Read_Gyro_Vec3();

void IMU_StartReadGyroIT(uint8_t * data);
Vec3 IMU_ConvertGyro(uint8_t * data);
void IMU_StartReadAccelIT(uint8_t * data);
Vec3 IMU_ConvertAccel(uint8_t * data);

void IMU_StartReadGyroAxisIT(uint8_t * data, uint8_t axis);
float IMU_ConvertGyroAxis(uint8_t * data);

Vec3 IMU_Read_Gyro_Vec3();
Vec3 IMU_Read_Accel_Vec3();

#endif /* IMU_H */
