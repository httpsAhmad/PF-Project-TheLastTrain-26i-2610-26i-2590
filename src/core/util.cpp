#include "util.h"
int clampInt(int val, int minVal, int maxVal){
    if (val < minVal) {
        return minVal;
    } else if (val > maxVal) {
        return maxVal;
    } else {
        return val;
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
void trimWhitespace(char* str){
    int start = 0;
    while (str[start] != '\0' && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n')) {
        start++;
    }
    int end = start;
    while (str[end] != '\0') {
        end++;
    }
    end--;
    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n')) {
        end--;
    }
    int newLength = end - start + 1;
    for (int i = 0; i < newLength; i++) {
        str[i] = str[start + i];
    }
    str[newLength] = '\0';
}
// Primitive helper utilities, clamping, and screen control
