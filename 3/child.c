#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <wait.h>
#include <semaphore.h>
#include <sys/mman.h>

#define SHM_SIZE 4096

int main(int argc, const char **argv) {
    bool first = argv[1][0] - '0';

    int shm = shm_open(argv[2], O_RDWR, 0);
    if (shm == -1) {
		const char msg[] = "error: failed to open SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}
    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
		const char msg[] = "error: failed to map SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}
    
    sem_t *sem = sem_open(argv[3], O_RDWR);
	if (sem == SEM_FAILED) {
		const char msg[] = "error: failed to open semaphore\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}
    int *stage = (int *)(shm_buf + sizeof(uint32_t));
    uint32_t *len = (uint32_t *)shm_buf;
    char *text = shm_buf + sizeof(uint32_t) + sizeof(int);
    char nbuf[4096];
    while(1) {
        // преобразить buf
        sem_wait(sem);
        if(first) { // child 1
            if(*stage == 1) {
                for(size_t i = 0; i < *len; ++i) text[i] = tolower(text[i]);
                *stage = 2; // дальше child2
            }
        } else { // child 2
            size_t nsz = 0;
            for(size_t i = 0; i < *len; ++i) {
                if(text[i] != ' ' || i == *len - 1 || text[i + 1] != ' ') {
                    nbuf[nsz++] = text[i];
                }
            }
            *len = nsz;
            memcpy(text, nbuf, *len);
            *stage = 3;
        }
        sem_post(sem);
    }
    return 0;
}