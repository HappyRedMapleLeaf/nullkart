#include "two_wheel_tracker.h"

#include <math.h>

void TwoWheelTracker_Init(TwoWheelTracker * tracker) {
    tracker->position.x = 0.0;
    tracker->position.y = 0.0;
    tracker->heading = 0.0;

    tracker->k1 = 2.0f * M_PI * tracker->wheel_radius / tracker->ticks_per_rev;
}

// maybe just directly keep track of ticks instead of using rate?
void TwoWheelTracker_Update(TwoWheelTracker * tracker, float dt_s) {
    float dL = tracker->left_wheel->rate_filtered * dt_s * tracker->k1;
    float dR = tracker->right_wheel->rate_filtered * dt_s * tracker->k1;
    float d_theta = (dR - dL) / tracker->track_width;

    float dx, dy;
    float cos_theta = cosf(tracker->heading);
    float sin_theta = sinf(tracker->heading);

    if (fabs(d_theta) < 1e-8) {
        // straight line motion
        dx = (dL + dR) / 2.0f * cos_theta;
        dy = (dL + dR) / 2.0f * sin_theta;
    } else {
        float r = (dL + dR) / (-2.0f * d_theta); // radius of curvature

        float r_d_theta = r * d_theta; // arc length
        float d_theta_half = d_theta / 2.0f;
        float k2 = 1 - d_theta * d_theta / 6.0f;

        // pray that I did my taylor approx right
        dx = r_d_theta * ( sin_theta * d_theta_half - cos_theta * k2);
        dy = r_d_theta * (-cos_theta * d_theta_half - sin_theta * k2);
    }

    tracker->position.x += dx;
    tracker->position.y += dy;
    tracker->heading += d_theta;
}