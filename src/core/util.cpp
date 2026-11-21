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
bool stringEquals(const char* a, const char* b){
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return false;
        }
        i++;
    }
    return a[i] == b[i];
}
// Primitive helper utilities, clamping, and screen control
