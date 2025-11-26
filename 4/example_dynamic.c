#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
// #include "lib.h"

typedef float (*pi_t)(int k);
typedef char* (*conv_t)(int x);

char *paths[] = {"./libown1.so", "./libown2.so"};

pi_t pi_func;
conv_t conv_func;
void * lib = NULL;

int open_and_init(int ind) {
    if(lib) dlclose(lib);
    lib = dlopen(paths[ind], RTLD_LAZY);
    pi_func = dlsym(lib, "pi");
    conv_func = dlsym(lib, "convert");
    if(!pi_func || !conv_func) return -1;
    return 0;
}

int main() {
    int choise;
    printf("Введите номер вызываемой функции а потом аргументы\nПереключить реалзиации библиотек 0\nДля выхода -1\n");  
    int ind = 0;
    if(open_and_init(ind) != 0) {
        printf("Ошибка при открытии или чтении функций из библиотеки\n");
        return 1;
    }
    scanf("%d", &choise);
    while(choise != -1) {
        int arg;
        if(!choise) {
            if(open_and_init(ind = !ind) != 0) {
                printf("Ошибка при открытии или чтении функций из библиотеки\n");
                return 1;
            }
        } if(choise == 1) {
            scanf("%d", &arg);
            printf("%s\n", conv_func(arg));
        } if(choise == 2) {
            scanf("%d", &arg);
            printf("%f\n", pi_func(arg));
        }
        scanf("%d", &choise);
    }
    dlclose(lib);
    return 0;  
}