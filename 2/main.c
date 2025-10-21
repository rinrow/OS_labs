#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int row, column;
} MArg;

typedef struct {
    int st, end;
} TArg;


#define N 1000
#define M 1000
// matr[i][j][0] - real part of number in i row and j column
// matr[i][j][1] - compl part of i number in row and j column
int matr1[N][M][2], matr2[M][N][2], res[N][N][2];
MArg args[N * M];

void calc(MArg *arg) {
    int r = arg->row, c = arg->column;
    int *v1, *v2, *vres; 
    for(int i = 0; i < M; ++i) {
        // matr1[r][i] * matr2[i][c];
        // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
        // a = matr1[r][i][0], b = matr1[r][i][1];
        // c = matr2[i][c][0], d = matr2[i][c][1];
        v1 = matr1[r][i], v2 = matr2[i][c];
        vres = res[r][c];
        vres[0] = v1[0] * v2[0] - v1[1] * v2[1];
        vres[1] = v1[0] * v2[1] + v1[1] * v2[0];
    }
}

void consistent() {
    for(int i = 0; i < N * M; ++i) {
        calc(args + i);
    }
}

static void *work(void *_arg) {
    TArg *arg = (TArg *)_arg;
    int st = arg->st, end = arg->end;
    for(int i = st; i < end; i++) {
        calc(args + i);
    }
    return NULL;
}

void paralel(int tcnt) {
	pthread_t *threads = malloc(tcnt * sizeof(pthread_t));
    int totTasks = N * M;
    int taskPerTread = totTasks / tcnt, rem = totTasks % tcnt;
    int curCnt, prevI = 0;
    for(int i = 0; i < tcnt; ++i) {
        curCnt = taskPerTread;
        if(rem) {
            ++curCnt;
            --rem;
        }
        TArg t = {prevI, prevI += curCnt};
        pthread_create(threads + i, NULL, work, &t);
    }
    for(int i = 0; i < tcnt; ++i) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
}

double getDif(struct timespec start, struct timespec end) {
    return 1e3 * ((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
}

int main() {
    for(int r = 0; r < N; ++r) {
        for(int c = 0; c < M; ++c) {
            args[r * M + c] = (MArg){r, c};
            matr1[r][c][0] = rand() % N;
            matr1[r][c][1] = rand() % N;
        }
    }
    for(int r = 0; r < M; ++r) {
        for(int c = 0; c < N; ++c) {
            matr2[r][c][0] = rand() % N;
            matr2[r][c][1] = rand() % N;
        }
    }

    struct timespec st, end;
    size_t tn = sysconf(_SC_NPROCESSORS_ONLN);
    double ts, curT;
    printf("Total thread count %lu\n\n", tn);
    printf("First matrix size (%d * %d), second matrix size (%d * %d)\n\n", N, M, M, N);
    {
    // experiment consistent
    printf("===== experiment consistent =====\n");
    clock_gettime(CLOCK_MONOTONIC, &st);
    consistent();
    clock_gettime(CLOCK_MONOTONIC, &end);
    ts = getDif(st, end);
    curT = getDif(st, end);
    printf("Time : %lf ms. Acceleration %lf . Efficiency %lf\n", curT, ts / curT, ts / curT / 1);
    printf("\n\n");

    // experiment Paralel <= tn
    printf("===== experiment Paralel threads < logic Thread =====\n");
    for(int i = 1; i < tn; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &st);
        paralel(i);
        clock_gettime(CLOCK_MONOTONIC, &end);
        curT = getDif(st, end);
        printf("ThreadsCount %d . Time : %lf ms. Acceleration %lf . Efficiency %lf\n", 
                i, curT, ts / curT, ts / curT / i);
    }
    printf("\n\n");

    printf("===== experiment threads = logic threads =====\n");
    clock_gettime(CLOCK_MONOTONIC, &st);
    paralel(tn);
    clock_gettime(CLOCK_MONOTONIC, &end);
    curT = getDif(st, end);
    printf("ThreadsCount %d . Time : %lf ms. Acceleration %lf . Efficiency %lf\n", 
                tn, curT, ts / curT, ts / curT / tn);
    printf("\n\n");

    int bigCnt[3] = {16, 32, 1024};
    printf("===== experiment threads > logic threads =====\n");
    for(int i = 0; i < 3; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &st);
        paralel(bigCnt[i]);
        clock_gettime(CLOCK_MONOTONIC, &end);
        curT = getDif(st, end);
        printf("ThreadsCount %d . Time : %lf ms. Acceleration %lf . Efficiency %lf\n", 
                bigCnt[i], curT, ts / curT, ts / curT / bigCnt[i]);
    }
    printf("\n\n");
    }
    
    return 0;
}