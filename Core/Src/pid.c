#include "pid.h"

float updatePID(PID_Controller * controller, float error, float elapsedTime) {
    float deriv_error = (error - controller->prev_error) / elapsedTime;
    controller->integral_error += error;

    // handle max integral. expect error values to be very large
    // do filtering on derivative
    // handle max output value

    return (controller->kp * error +
            controller->ki * controller->integral_error + 
            controller->kd * deriv_error
    );
}