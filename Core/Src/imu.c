#include "imu.h"

#include <math.h>

#define TIMEOUT_MS                      1
#define I2C_ADDRESS                     0x68

#define REG_BANK_SEL                    0x7F

// USER BANK 0 REGISTERS
#define WHO_AM_I                        0x00
#define USER_CTRL                       0x03
#define PWR_MGMT_1                      0x06

#define TEMP_LSB_REGISTER               0x3A
#define TEMP_MSB_REGISTER               0x39

#define ACCEL_X_LSB_REGISTER            0x2E
#define ACCEL_X_MSB_REGISTER            0x2D
#define ACCEL_Y_LSB_REGISTER            0x30
#define ACCEL_Y_MSB_REGISTER            0x2F
#define ACCEL_Z_LSB_REGISTER            0x32
#define ACCEL_Z_MSB_REGISTER            0x31

#define GYRO_X_LSB_REGISTER             0x34
#define GYRO_X_MSB_REGISTER             0x33
#define GYRO_Y_LSB_REGISTER             0x36
#define GYRO_Y_MSB_REGISTER             0x35
#define GYRO_Z_LSB_REGISTER             0x38
#define GYRO_Z_MSB_REGISTER             0x37

// USER BANK 1 REGISTERS

// USER BANK 2 REGISTERS
#define B2_ACCEL_CONFIG                 0x14
#define B2_GYRO_CONFIG_1                0x01

// USER BANK 3 REGISTERS

extern I2C_HandleTypeDef hi2c3;

uint8_t user_bank = 0;
uint8_t accel_fs  = 0b00;
uint8_t gyro_fs  = 0b00;

HAL_StatusTypeDef I2C_Read_Byte(uint16_t MemAddress, uint8_t *data) {
   uint8_t data_array[1];
   HAL_StatusTypeDef res = HAL_I2C_Mem_Read(&hi2c3, (I2C_ADDRESS << 1) | 1, MemAddress, I2C_MEMADD_SIZE_8BIT, data_array, 1, TIMEOUT_MS);
   *data = data_array[0];
   return res;
}

HAL_StatusTypeDef I2C_Write_Byte(uint16_t MemAddress, uint8_t data) {
   uint8_t data_array[1] = {data};
   return HAL_I2C_Mem_Write(&hi2c3, I2C_ADDRESS << 1, MemAddress, I2C_MEMADD_SIZE_8BIT, data_array, 1, TIMEOUT_MS);
}

HAL_StatusTypeDef I2C_Read_Bytes(uint16_t MemAddress, uint8_t *data, uint16_t Size) {
   return HAL_I2C_Mem_Read(&hi2c3, (I2C_ADDRESS << 1) | 1, MemAddress, I2C_MEMADD_SIZE_8BIT, data, Size, TIMEOUT_MS);
}

HAL_StatusTypeDef I2C_Write_Bytes(uint16_t MemAddress, uint8_t *data, uint16_t Size) {
   return HAL_I2C_Mem_Write(&hi2c3, I2C_ADDRESS << 1, MemAddress, I2C_MEMADD_SIZE_8BIT, data, Size, TIMEOUT_MS);
}

void IMU_Change_User_Bank(uint8_t bank) {
   if (user_bank == bank) {
       return;
   }
   if (bank > 3) {
       return;
   }
   I2C_Write_Byte(REG_BANK_SEL, bank << 4);
   user_bank = bank;
}

void IMU_Set_Accel_Range(uint8_t accel_fs_new) {
   // 0b00 = +/- 4g
   // 0b01 = +/- 8g
   // 0b10 = +/- 16g
   // 0b11 = +/- 30g
   if (accel_fs_new == accel_fs) {
       return;
   }
   if (accel_fs_new > 3) {
       return;
   }
   IMU_Change_User_Bank(2);
   // set bits 2:1. Default value is 0b00000001
   I2C_Write_Byte(B2_ACCEL_CONFIG, (accel_fs_new << 1) | (0b00000001 & 0b11111001));
   accel_fs = accel_fs_new;
}

void IMU_Set_Gyro_Range(uint8_t gyro_fs_new) {
   // 00 = +/- 500 dps
   // 01 = +/- 1000 dps
   // 10 = +/- 2000 dps
   // 11 = +/- 4000 dps
   if (gyro_fs_new == gyro_fs) {
       return;
   }
   if (gyro_fs_new > 3) {
       return;
   }
   IMU_Change_User_Bank(2);
   // set bits 2:1. Default value is 0b00000001
   I2C_Write_Byte(B2_GYRO_CONFIG_1, (gyro_fs_new << 1) | (0b00000001 & 0b11111001));
   gyro_fs = gyro_fs_new;
}

uint8_t IMU_WhoAmI() {
   // IMU_Change_User_Bank(0);
   uint8_t res;
   I2C_Read_Byte(WHO_AM_I, &res);
   return res;
}

void IMU_WhoAmIIT(uint8_t * res) {
   HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, res, 1);
}

double IMU_Read_Accel(uint8_t axis) {
   IMU_Change_User_Bank(0);

   uint8_t data[2];

   if (axis == AXIS_X) {
       I2C_Read_Bytes(ACCEL_X_MSB_REGISTER, data, 2);
   } else if (axis == AXIS_Y) {
       I2C_Read_Bytes(ACCEL_Y_MSB_REGISTER, data, 2);
   } else if (axis == AXIS_Z) {
       I2C_Read_Bytes(ACCEL_Z_MSB_REGISTER, data, 2);
   }

   int16_t g = (data[0] << 8) | data[1];

   return g * (1 << accel_fs) / 8192.0;
}

double IMU_Read_Gyro(uint8_t axis) {
   IMU_Change_User_Bank(0);

   uint8_t data[2];

   if (axis == AXIS_X) {
       I2C_Read_Bytes(GYRO_X_MSB_REGISTER, data, 2);
   } else if (axis == AXIS_Y) {
       I2C_Read_Bytes(GYRO_Y_MSB_REGISTER, data, 2);
   } else if (axis == AXIS_Z) {
       I2C_Read_Bytes(GYRO_Z_MSB_REGISTER, data, 2);
   }

   int16_t rate = (data[0] << 8) | data[1];

   double gyro_sensitivity = 65.5;
   if (gyro_fs == 1) {
       gyro_sensitivity = 32.8;
   } else if (gyro_fs == 2) {
       gyro_sensitivity = 16.4;
   } else if (gyro_fs == 3) {
       gyro_sensitivity = 8.2;
   }
   return rate / gyro_sensitivity;
}

double IMU_Read_Temp() {
   IMU_Change_User_Bank(0);

   uint8_t data[2];
   I2C_Read_Bytes(TEMP_MSB_REGISTER, data, 2);

   int16_t temp = (data[0] << 8) | data[1];

   return (temp - 21.0)/333.87 + 21.0;
}

void IMU_Init() {
   HAL_Delay(100);
   IMU_Change_User_Bank(0);
   // default startup is 0x41, we want to disable sleep
   // which is the 2nd byte so it becomes 0x01
   I2C_Write_Byte(PWR_MGMT_1, 0x01);
   HAL_Delay(40);
   // IMU_Set_Accel_Range(0b01);
   // IMU_Set_Gyro_Range(0b10);
}

void IMU_Reset() {
   IMU_Change_User_Bank(0);
   I2C_Write_Byte(USER_CTRL, 0b00000100);  // reset SRAM
   I2C_Write_Byte(PWR_MGMT_1, 0b10000001); // reset all registers
}

Vec3 IMU_Read_Accel_Vec3() {
   uint8_t data[6];
   I2C_Read_Bytes(ACCEL_X_MSB_REGISTER, data, 6);

   return IMU_ConvertAccel(data);
}

void IMU_StartReadAccelIT(uint8_t * data) {
   HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, ACCEL_X_MSB_REGISTER, I2C_MEMADD_SIZE_8BIT, data, 6);
}

Vec3 IMU_ConvertAccel(uint8_t * data) {
   int16_t x = (data[0] << 8) | data[1];
   int16_t y = (data[2] << 8) | data[3];
   int16_t z = (data[4] << 8) | data[5];

   double factor = (1 << accel_fs) / 8192.0 * G_TO_MM_S_2;

   return (Vec3) {
       x * factor,
       y * factor,
       z * factor
   };
}

Vec3 IMU_Read_Gyro_Vec3() {
   uint8_t data[6];
   I2C_Read_Bytes(GYRO_X_MSB_REGISTER, data, 6);

   return IMU_ConvertGyro(data);
}

void IMU_StartReadGyroAxisIT(uint8_t * data, uint8_t axis) {
    if (axis == AXIS_X) {
        HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, GYRO_X_MSB_REGISTER, I2C_MEMADD_SIZE_8BIT, data, 2);
    } else if (axis == AXIS_Y) {
        HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, GYRO_Y_MSB_REGISTER, I2C_MEMADD_SIZE_8BIT, data, 2);
    } else if (axis == AXIS_Z) {
        HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, GYRO_Z_MSB_REGISTER, I2C_MEMADD_SIZE_8BIT, data, 2);
    }
}

float IMU_ConvertGyroAxis(uint8_t * data) {
    int16_t a = (data[0] << 8) | data[1];
 
    double gyro_sensitivity = 65.5;
    if (gyro_fs == 1) {
        gyro_sensitivity = 32.8;
    } else if (gyro_fs == 2) {
        gyro_sensitivity = 16.4;
    } else if (gyro_fs == 3) {
        gyro_sensitivity = 8.2;
    }
 
    double factor = (M_PI / gyro_sensitivity) / 180;
 
    return a * factor;
}

void IMU_StartReadGyroIT(uint8_t * data) {
   HAL_I2C_Mem_Read_IT(&hi2c3, (I2C_ADDRESS << 1) | 1, GYRO_X_MSB_REGISTER, I2C_MEMADD_SIZE_8BIT, data, 6);
}

Vec3 IMU_ConvertGyro(uint8_t * data) {
   int16_t x = (data[0] << 8) | data[1];
   int16_t y = (data[2] << 8) | data[3];
   int16_t z = (data[4] << 8) | data[5];

   double gyro_sensitivity = 65.5;
   if (gyro_fs == 1) {
       gyro_sensitivity = 32.8;
   } else if (gyro_fs == 2) {
       gyro_sensitivity = 16.4;
   } else if (gyro_fs == 3) {
       gyro_sensitivity = 8.2;
   }

   double factor = (M_PI / gyro_sensitivity) / 180;

   return (Vec3) {
       x * factor,
       y * factor,
       z * factor
   };
}
