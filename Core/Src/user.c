#include "user.h"
#include "main.h"
#include "common.h"
#include <stdbool.h>
#include <math.h>

#include "math_utils.h"
#include "globals.h"
#include "imu.h"
#include "pid.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;

void SoftReset() {
    i2cEnable = false;

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    HAL_I2C_DeInit(&hi2c3);
    HAL_Delay(100);
    HAL_I2C_Init(&hi2c3);
    HAL_Delay(100);
    IMU_Reset();
    IMU_Init();
    HAL_Delay(100);

    uint8_t who = IMU_WhoAmI();
    // error
    if (who != 0xE1) {
        while (1) {}
    }

    // some weird shit happened here idk
    int32_t init_us = TIM5->CNT;
    uint32_t cal_cycle = 1;
    gyro_drift = IMU_Read_Gyro_Vec3();
    Vec3 average_accel = IMU_Read_Accel_Vec3();

    // calibration for 100ms
    while (TIM5->CNT - init_us < 84000*100) {
        cal_cycle++;
        Vec3 accel = IMU_Read_Accel_Vec3();
        average_accel = vec3_add(average_accel, accel);
        Vec3 gyro = IMU_Read_Gyro_Vec3();
        gyro_drift = vec3_add(gyro_drift, gyro);
    }
    average_accel = vec3_scale(average_accel, 1.0 / cal_cycle);
    gyro_drift = vec3_scale(gyro_drift, 1.0 / cal_cycle);

    accel_drift = vec3_sub(average_accel, (Vec3) {0, 0, G_TO_MM_S_2});

    i2cEnable = true;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

void startReadIMU() {
    IMU_StartReadGyroIT(imu_gyro_data);
    readStatusIMU = 1; // indicate reading gyro
}

void setMotorPower(float power) {
    if (power > 1.0) {
        power = 1.0;
    }
    if (power < -1.0) {
        power = -1.0;
    }
    uint16_t pwm = (uint16_t) (fabs(power)*4200.0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pwm);
    if (power > 0) {
        HAL_GPIO_WritePin(MOTOR1_FWD_GPIO_Port, MOTOR1_FWD_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR1_REV_GPIO_Port, MOTOR1_REV_Pin, GPIO_PIN_RESET);
    }
    // coast if power == 0
    if (power < 0) {
        HAL_GPIO_WritePin(MOTOR1_REV_GPIO_Port, MOTOR1_REV_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR1_FWD_GPIO_Port, MOTOR1_FWD_Pin, GPIO_PIN_RESET);
    }
}

void updateRateAvg(float newSample) {
    // Remove the oldest sample from the sum
    rateAvgSum -= rateAvgBuffer[rateAvgIdx];
    rateAvgBuffer[rateAvgIdx] = newSample;
    rateAvgSum += newSample;
    rateAvgIdx = (rateAvgIdx + 1) % RATE_AVG_BUF_SIZE;
    rateAvg = rateAvgSum / RATE_AVG_BUF_SIZE;
}

// I think integer wrapping does this for me but uhhhhhhh
uint32_t timerDiffWrapped(uint32_t a, uint32_t b) {
    if (a < b) {
        return 0xFFFFFFFF - (b - a) + 1;
    } else {
        return a - b;
    }
}

// quick utility function
// checks whether delay has passed since prevTimer
// if so, prevTimer is reset
bool compareTimer(uint32_t * prevTimer, uint32_t delay) {
    bool delayReached = timerDiffWrapped(TIM5->CNT, *prevTimer) > delay;
    if (delayReached) {
        *prevTimer = TIM5->CNT;
    }
    return delayReached;
}

// checks whether condition is met and delay has passed since prevTimer
// if both are true, prevTimer is reset 
bool compareTimerCond(uint32_t * prevTimer, uint32_t delay, bool condition) {
    bool delayReachedAndCondMet = timerDiffWrapped(TIM5->CNT, *prevTimer) > delay && condition;
    if (delayReachedAndCondMet) {
        *prevTimer = TIM5->CNT;
    }
    return delayReachedAndCondMet;
}

void user_init() {
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start(&htim5);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);

    SoftReset();
    startReadIMU();
}

// float thing = 0.0;
// bool direction = false;

void user_loop() {
    // imu up to every 10 ms
    if (timerDiffWrapped(TIM5->CNT, prev_time_imu_rx) > 84000*5 && readStatusIMU == 0) {
        double delta_t = timerDiffWrapped(TIM5->CNT, prev_time_imu_rx) / 84000000.0;
        prev_time_imu_rx = TIM5->CNT;

        Vec3 gyro = vec3_sub(IMU_ConvertGyro(imu_gyro_data), gyro_drift);
        Vec3 accel = vec3_sub(IMU_ConvertAccel(imu_accel_data), accel_drift);
        // Vec3 gyro = IMU_ConvertGyro(imu_gyro_data);
        // Vec3 accel = IMU_ConvertAccel(imu_accel_data);
        // Vec3 accel = {100000, 0, 0}; // force no fusion
        startReadIMU();
        // dir = vec4_from_vec3(accel);

        // https://ahrs.readthedocs.io/en/latest/filters/madgwick.html
        // note: they use wxyz but this uses xyzw so formulas will look different

        // quaternion derivative = 0.5 * (prev dir * angvel reading)
        Vec4 d_q = vec4_scale(vec4_mul(dir, vec4_from_vec3(gyro)), 0.5);

        if (fabs(vec3_mag(accel)/G_TO_MM_S_2 - 1) < 0.02) {
            // acceleration is within 2% of 1g, use fusion
            // otherwise only use gyro reading

            Vec3 accel_n = vec3_norm(accel);

            // transposed jacobian of error function
            Mat43 jacobian_t = {
                {-2*dir.z,  2*dir.w, -2*dir.x,  2*dir.y},
                { 2*dir.y,  2*dir.x,  2*dir.w,  2*dir.z},
                {       0, -4*dir.y, -4*dir.z,  0      }
            };

            Vec3 error = {
                2*(dir.y*dir.w - dir.x*dir.z) - accel_n.x,
                2*(dir.x*dir.y + dir.z*dir.w) - accel_n.y,
                2*(0.5 - dir.y*dir.y - dir.z*dir.z) - accel_n.z
            };

            Vec4 gradient = mat43_mul_vec3(jacobian_t, error);

            // correct quaternion derivative by beta * _____
            d_q = vec4_sub(d_q, vec4_scale(vec4_norm(gradient), madgwick_beta));
        }

        // double delta_t = 0.005;
        // integrate direction by incrementing by delta_t * derivative of direction
        dir = vec4_add(dir, vec4_scale(d_q, delta_t));

        theta = tilt_angle(dir);
    }

    // rx up to every 50 ms
    if (compareTimerCond(&prev_time_UART_rx, 84000*50, rxed)) {
        rxed = false;
        HAL_UART_Receive_IT(&huart2, rxData, 4);

        // update target based on received data
        // no need for syncing, we assume mcu is on before host starts sending things
        // so first byte read will be the first byte of a float
        memcpy(&target_rate, rxData, 4);
    }

    // run every 200 ms
    if (compareTimerCond(&prev_time_tx, 84000*200, txReady)) {
        txReady = false;
        // float toSend[4] = {dir.x, dir.y, dir.z, dir.w};
        float toSend[4] = {target_rate, rateAvg, theta, power};

        uint8_t data[20];
        memset(data, 0xFF, 4);
        memcpy(data+4, toSend, 16);
        HAL_UART_Transmit_IT(&huart2, data, 20);
    }
    
    float elapsed_ms = (TIM5->CNT - prev_time_pid) / 84000.0;
    prev_time_pid = TIM5->CNT;
    power = -updatePID(&flywheelMotor, rateAvg - target_rate, elapsed_ms);
    setMotorPower(power);

    // this will be used for angle control eventually
    // error = angle since target = 0
    // if error is always 0, target_rate should not change.
    // if error is nonzero, target_rate must increase based on the size of error.
    // therefore the output of the PID contorller is the change in target_rate
    // this feels cursed. not sure why yet
    target_rate -= updatePID(&angleController, theta, elapsed_ms);
    if (target_rate > 1000) {
        target_rate = 1000;
    } else if (target_rate < -1000) {
        target_rate = -1000;
    }

    HAL_Delay(1); // crucially important
    // if the loop is too fast some weird race condition thing happens and the uart transmission messes up
    // will investigate further later (maybe)
}

// currently every 10ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
    if (htim == &htim2) {
        tick_rate = (ticks - prev_ticks)/0.01;
        prev_ticks = ticks;

        // if (fabs(tick_rate) > 0.0) {
        //     HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        // } else {
        //     HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        // }

        updateRateAvg(tick_rate);
    } else {
        __NOP();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    rxed = true;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == ENC1_A_BLUE_Pin) {
        if (HAL_GPIO_ReadPin(ENC1_A_BLUE_GPIO_Port, ENC1_A_BLUE_Pin) == HAL_GPIO_ReadPin(ENC1_B_WHITE_GPIO_Port, ENC1_B_WHITE_Pin)) {
            // going backwards if falling edge on channel B low
            // or rising edge on channel B high
            ticks--;
        } else {
            // going forwards if rising edge on channel B low
            // or falling edge on channel B high
            ticks++;
        }
    } else if (GPIO_Pin == ENC1_B_WHITE_Pin) {
        if (HAL_GPIO_ReadPin(ENC1_A_BLUE_GPIO_Port, ENC1_A_BLUE_Pin) == HAL_GPIO_ReadPin(ENC1_B_WHITE_GPIO_Port, ENC1_B_WHITE_Pin)) {
            // going forwards if rising edge on channel A low
            // or falling edge on channel A high
            ticks++;
        } else {
            // going backwards if falling edge on channel A low
            // or rising edge on channel A high
            ticks--;
        }
    } else {
        __NOP();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        txReady = true;
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C3) {
        if (readStatusIMU == 1) { // reading gyro
            IMU_StartReadAccelIT(imu_accel_data);
            readStatusIMU = 2;
        } else if (readStatusIMU == 2) {
            readStatusIMU = 0;
        }
    }
}
