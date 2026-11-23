#include "util.h"
#include <iostream>
using namespace std;
int clampInt(int val, int minVal, int maxVal)
{
    if (val < minVal)
    {
        return minVal;
    }
    else if (val > maxVal)
    {
        return maxVal;
    }
    else
    {
        return val;
    }
}
bool stringEquals(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return false;
        }
        i++;
    }
    return a[i] == b[i];
}
void trimWhitespace(char *str)
{
    int start = 0;
    while (str[start] != '\0' && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r'))
    {
        start++;
    }
    int end = start;
    while (str[end] != '\0')
    {
        end++;
    }
    end--;
    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r'))
    {
        end--;
    }
    int newLength = end - start + 1;
    for (int i = 0; i < newLength; i++)
    {
        str[i] = str[start + i];
    }
    str[newLength] = '\0';
}
void printHeader(const char *title)
{
    for (int i = 0; i < 30; i++)
    {
        cout << "=";
    }
    cout << endl;
    cout << " " << title << endl;
    for (int i = 0; i < 30; i++)
    {
        cout << "=";
    }
    cout << endl;
}
void clearScreen()
{
    for (int i = 0; i < 40; i++)
    {
        cout << "\n";
    }
}
// Primitive helper utilities, clamping, and screen control