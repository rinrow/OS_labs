#include "lib.h"

// в троичную
char *convert(int x) {
    char *res = (char *)calloc(33, 1);
    char tmp[32];
    char *ptr = tmp, *ptr_res = res;
    if(!x) *ptr_res++ = '0';
    int neg = 0;
    if(x < 0) {
        neg = 1;
        x = -x;
    }   
    while(x) {
        *ptr++ = (x % 3) + '0';
        x /= 3;
    }
    if(neg) *ptr_res++ = '-';
    while(ptr != tmp) {
        *ptr_res++ = *--ptr;
    }
    return res;
}

float pi(int k) {
    float res = 1;
    for(int i = 1; i <= k; i++) {
        res *= 4. * i * i  / (4 * i * i - 1);
    }
    res *= 2;
    return res;
}