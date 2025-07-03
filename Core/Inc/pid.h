#ifndef PID_H
#define PID_H

typedef struct {
    const float kp;
    const float ki;
    const float kd;

    float integral_error;
    float prev_error;
} PID_Controller;

float updatePID(PID_Controller * controller, float error, float elapsedTime);

#endif