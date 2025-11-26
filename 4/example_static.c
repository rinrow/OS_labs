#include <stdio.h>
#include "lib.h"

int main() {
    int choise;
    printf("Введите номер вызываемой функции а потом аргументы\nДля выхода 0\n\n");  
    scanf("%d", &choise);
    while(choise) {
        int arg;
        scanf("%d", &arg);
        if(choise == 1) {
            printf("%s\n", convert(arg));
        }
        if(choise == 2) {
            printf("%f\n", pi(arg));
        }
        scanf("%d", &choise);
    }
    return 0;
}