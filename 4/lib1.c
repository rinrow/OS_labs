#include "lib.h"

// в двоичную
char *convert(int x) {
    char *res = (char *)calloc(34, 1);
    char tmp[32];
    char *ptr = tmp, *ptr_res = res;
    if(!x) *ptr_res++ = '0';
    int neg = 0;
    if(x < 0) {
        neg = 1;
        x = -x;
    }   
    while(x) {
        *ptr++ = (x % 2) + '0';
        x /= 2;
    }
    if(neg) *ptr_res++ = '-';
    while(ptr != tmp) {
        *ptr_res++ = *--ptr;
    }
    return res;
}

float pi(int k) {
    float res = 0;
    for(int i = 0; i < k; i++) {
        res += (i & 1 ? -1.: 1.) / (2 * i + 1); 
    }
    res *= 4;
    return res;
}