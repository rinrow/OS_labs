#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
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

char CHILD_PROGRAM_NAME[] = "child";

int main() {
    char shm_name[256], sem_name[256];
    snprintf(shm_name, 256, "client_shm%d", getpid());
    snprintf(sem_name, 256, "client_sem%d", getpid());

    int shm = shm_open(shm_name, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if(shm == -1) {
        char msg[] = "error:failed to create/open SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm, SHM_SIZE) == -1) {
        const char msg[] = "error: failed to resize SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }

    char* shm_buf = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);
    if(shm_buf == MAP_FAILED) {
        const char msg[] = "error: failed to map SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }

    sem_t *sem = sem_open(sem_name, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
    if (sem == SEM_FAILED) {
		const char msg[] = "error: failed to create semaphore\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}
    
    int *stage = (int *)(shm_buf + sizeof(uint32_t));
    uint32_t *len = (uint32_t *)shm_buf;
    char *text = shm_buf + sizeof(uint32_t) + sizeof(int);
    *stage = 0; *len = 0;

    const pid_t child1 = fork();
    if(child1 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } else if(child1 == 0) {
        char *argv[] = {"child", "1", shm_name, sem_name, NULL};
        execv("./child", argv);
        const char msg[] = "error: failed to exec\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } 

    const pid_t child2 = fork();
    if(child2 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } else if(child2 == 0) {
        char *argv[] = {"child", "0", shm_name, sem_name, NULL};
        execv("./child", argv);
        const char msg[] = "error: failed to exec\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } 

    // client
    pid_t pid = getpid(); 
    size_t bytes;
    while(true) {
        char buf[SHM_SIZE - sizeof(uint32_t)];
        bytes = read(STDIN_FILENO, buf, sizeof(buf));
        if(*len == -1) {
            const char msg[] = "error: failed to read from standard input\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			_exit(EXIT_FAILURE);
        }
        sem_wait(sem);
        *len = bytes;
        if(*len > 0) {
            memcpy(text, buf, *len);
        } else {
            *len = UINT32_MAX;
            sem_post(sem);
            break;
        }
        *stage = 1; // дальше обрабатывать будет child1
        sem_post(sem);

        // вывод после child2
        // ждем пока child2 не закончит работу
        while (true) {
            sem_wait(sem);
            if(*stage == 3) {
                // child2 законил обрабоотку
                write(STDOUT_FILENO, text, *len);
                *stage = 0;
                sem_post(sem);
                break;
            }
            sem_post(sem);
        }
    }
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

	sem_unlink(sem_name);
	sem_close(sem);
	munmap(shm_buf, SHM_SIZE);
	shm_unlink(shm_name);
	close(shm);
    return 0;
}
