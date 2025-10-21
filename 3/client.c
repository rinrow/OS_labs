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

char CHILD_PROGRAM_NAME[] = "child";

int main() {
    int shm_name[256], sem_name[256];
    snprintf(shm_name, 256, "client_shm%s", getpid());
    snprintf(shm_name, 256, "client_sem%s", getpid());

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
    *stage = 0;

    const pid_t child1 = fork();
    if(child1 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } else if(child1 == 0) {
        char *argv[] = {"child", "1", shm_name, sem_name, NULL};
        execv("./client", argv);
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
        execv("./client", argv);
        const char msg[] = "error: failed to exec\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } 
    // client
    pid_t pid = getpid(); 
    char msg[64];
    const int32_t length = snprintf(msg, sizeof(msg),
        "%d: I'm a Parent\n", pid);
    write(STDOUT_FILENO, msg, length);
    bool run = 1;
    while(run) {
        char buf[SHM_SIZE - sizeof(uint32_t)];
        size_t bytes = read(STDIN_FILENO, buf, sizeof(buf));
        if(bytes == -1) {
            const char msg[] = "error: failed to read from standard input\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			_exit(EXIT_FAILURE);
        }
        sem_wait(sem);
        if(bytes > 0) {
            *len = bytes;
            memcpy(text, buf, bytes);
        }  else {
            *len = UINT32_MAX;
            run = 0;
        }
        *stage = 1; // дальше обрабатывать будет child1
        sem_post(sem);

        // вывод после child2
        // ждем пока child2 не закончит работу
        while (1) {
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
    return 0;
}
