#ifndef TWO_WHEEL_TRACKER_H
#define TWO_WHEEL_TRACKER_H

#include "dcmotor_drv.h"
#include "math_utils.h"

typedef struct two_wheel_tracker {
    DCMotor * left_wheel;
    DCMotor * right_wheel;
    float wheel_radius; // in meters
    float track_width; // in meters, distance between wheels
    int32_t ticks_per_rev;

    Vec2 position; // in meters
    float heading;

    float k1; // ticks to meters
} TwoWheelTracker;

void TwoWheelTracker_Init(TwoWheelTracker * tracker);

// maybe just directly keep track of ticks instead of using rate?
void TwoWheelTracker_Update(TwoWheelTracker * tracker, float dt_s);

#endif // TWO_WHEEL_TRACKER_H