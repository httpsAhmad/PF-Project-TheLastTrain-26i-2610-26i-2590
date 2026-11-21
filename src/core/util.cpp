#include "util.h"
int clampInt(int value, int minValue, int maxValue){
    if (value < minValue) {
        return minValue;
    } else if (value > maxValue) {
        return maxValue;
    } else {
        return value;
    }
}
// Primitive helper utilities, clamping, and screen control
