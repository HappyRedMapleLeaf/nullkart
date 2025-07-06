#include "common.h"

int8_t plusmod(int8_t a, int8_t b) {
    return ((a % b) + b) % b;
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