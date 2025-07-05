#include "user.h"
#include "main.h"
#include "common.h"
#include <stdbool.h>
#include <math.h>

#include "math_utils.h"
#include "globals.h"
// #include "imu_drv.h"
#include "pid.h"
#include "dcmotor_drv.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern UART_HandleTypeDef huart2;
// extern I2C_HandleTypeDef hi2c3;

// L PWM PB4 3/1; DIR PB5, PA10; ENC PC2, PC3
// R PWM PC7 3/2; DIR PC0, PA9; ENC PH0, PH1

DCMotor left_wheel = {
    .htim_pwm = &htim3,
    .pwm_channel = TIM_CHANNEL_1,
    .fwd_GPIO_port = L_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = L_DIR_1_Pin,
    .rev_GPIO_port = L_DIR_2_GPIO_Port,
    .rev_GPIO_pin = L_DIR_2_Pin,
};

DCMotor right_wheel = {
    .htim_pwm = &htim2,
    .pwm_channel = TIM_CHANNEL_2,
    .fwd_GPIO_port = R_DIR_1_GPIO_Port,
    .fwd_GPIO_pin = R_DIR_1_Pin,
    .rev_GPIO_port = R_DIR_2_GPIO_Port,
    .rev_GPIO_pin = R_DIR_2_Pin,
};

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
    HAL_TIM_Base_Start(&htim5);

    // SoftReset();
    // startReadIMU();
}

float power = 0.0;
bool up = false;

void user_loop() {
    // // imu up to every 10 ms
    // if (timerDiffWrapped(TIM5->CNT, prev_time_imu_rx) > 84000*5 && readStatusIMU == 0) {
    //     double delta_t = timerDiffWrapped(TIM5->CNT, prev_time_imu_rx) / 84000000.0;
    //     prev_time_imu_rx = TIM5->CNT;

    //     Vec3 gyro = vec3_sub(IMU_ConvertGyro(imu_gyro_data), gyro_drift);
    //     Vec3 accel = vec3_sub(IMU_ConvertAccel(imu_accel_data), accel_drift);
    //     // Vec3 gyro = IMU_ConvertGyro(imu_gyro_data);
    //     // Vec3 accel = IMU_ConvertAccel(imu_accel_data);
    //     // Vec3 accel = {100000, 0, 0}; // force no fusion
    //     startReadIMU();
    //     // dir = vec4_from_vec3(accel);

    //     // https://ahrs.readthedocs.io/en/latest/filters/madgwick.html
    //     // note: they use wxyz but this uses xyzw so formulas will look different

    //     // quaternion derivative = 0.5 * (prev dir * angvel reading)
    //     Vec4 d_q = vec4_scale(vec4_mul(dir, vec4_from_vec3(gyro)), 0.5);

    //     if (fabs(vec3_mag(accel)/G_TO_MM_S_2 - 1) < 0.02) {
    //         // acceleration is within 2% of 1g, use fusion
    //         // otherwise only use gyro reading

    //         Vec3 accel_n = vec3_norm(accel);

    //         // transposed jacobian of error function
    //         Mat43 jacobian_t = {
    //             {-2*dir.z,  2*dir.w, -2*dir.x,  2*dir.y},
    //             { 2*dir.y,  2*dir.x,  2*dir.w,  2*dir.z},
    //             {       0, -4*dir.y, -4*dir.z,  0      }
    //         };

    //         Vec3 error = {
    //             2*(dir.y*dir.w - dir.x*dir.z) - accel_n.x,
    //             2*(dir.x*dir.y + dir.z*dir.w) - accel_n.y,
    //             2*(0.5 - dir.y*dir.y - dir.z*dir.z) - accel_n.z
    //         };

    //         Vec4 gradient = mat43_mul_vec3(jacobian_t, error);

    //         // correct quaternion derivative by beta * _____
    //         d_q = vec4_sub(d_q, vec4_scale(vec4_norm(gradient), madgwick_beta));
    //     }

    //     // double delta_t = 0.005;
    //     // integrate direction by incrementing by delta_t * derivative of direction
    //     dir = vec4_add(dir, vec4_scale(d_q, delta_t));

    //     theta = tilt_angle(dir);
    // }

    // // rx up to every 50 ms
    // if (compareTimerCond(&prev_time_UART_rx, 84000*50, rxed)) {
    //     rxed = false;
    //     HAL_UART_Receive_IT(&huart2, rxData, 4);

    //     // update target based on received data
    //     // no need for syncing, we assume mcu is on before host starts sending things
    //     // so first byte read will be the first byte of a float
    //     memcpy(&target_rate, rxData, 4);
    // }

    // // run every 200 ms
    // if (compareTimerCond(&prev_time_tx, 84000*200, txReady)) {
    //     txReady = false;
    //     // float toSend[4] = {dir.x, dir.y, dir.z, dir.w};
    //     float toSend[4] = {target_rate, rateAvg, theta, power};

    //     uint8_t data[20];
    //     memset(data, 0xFF, 4);
    //     memcpy(data+4, toSend, 16);
    //     HAL_UART_Transmit_IT(&huart2, data, 20);
    // }
    
    // if (power < 0.0) {
    //     power = 0.0;
    //     up = true;
    // } else if (power > 1.0) {
    //     power = 1.0;
    //     up = false;
    // }
    // if (up) {
    //     power += 0.01;
    // } else {
    //     power -= 0.01;
    // }
    // DCMotor_SetPower(&left_wheel, power);
    // DCMotor_SetPower(&right_wheel, power);
    DCMotor_SetPower(&left_wheel, 1);
    DCMotor_SetPower(&right_wheel, 1);

    HAL_Delay(10); // crucially important
    // if the loop is too fast some weird race condition thing happens and the uart transmission messes up
    // will investigate further later (maybe)
}

// currently every 10ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
    DCMotor_UpdateEncoder(&left_wheel);
    DCMotor_UpdateEncoder(&right_wheel);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    rxed = true;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    DCMotor_EncoderCallback(&left_wheel, GPIO_Pin);
    DCMotor_EncoderCallback(&right_wheel, GPIO_Pin);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        txReady = true;
    }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    MFilt_I2CCallback(&mfilt, hi2c);
}
