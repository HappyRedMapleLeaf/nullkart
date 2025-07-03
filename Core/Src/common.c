#include "common.h"

int8_t plusmod(int8_t a, int8_t b) {
    return ((a % b) + b) % b;
}