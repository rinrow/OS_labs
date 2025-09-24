#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static char CHILD_PROGRAM_NAME[] = "child";

int main() {
    char progpath[1024];
	{
		// NOTE: Read full program path, including its name
		ssize_t len = readlink("/proc/self/exe", progpath,
		                       sizeof(progpath) - 1);
		if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		// NOTE: Trim the path to first slash from the end
		while (progpath[len] != '/')
			--len;

		progpath[len] = '\0';
	}
    // parent -> child1
    int pipe1[2];
    if(pipe(pipe1) == -1) {
        const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    // child2 -> parent
    int pipe2[2];
    if(pipe(pipe2) == -1) {
        const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    // child1 -> child2
    int pipe3[2];
    if(pipe(pipe3) == -1) {
        const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    }
    const pid_t child1 = fork();
    if(child1 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } else if(child1 == 0) {
        {
			pid_t pid = getpid(); // NOTE: Get child PID

			char msg[64];
			const int32_t length = snprintf(msg, sizeof(msg),
				"%d: I'm a child\n", pid);
			write(STDOUT_FILENO, msg, length);
		}
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);
        dup2(pipe3[1], STDOUT_FILENO);
        close(pipe3[1]);
        close(pipe3[0]);
        {
            char path[1024];
            snprintf(path, sizeof(path) - 1, "%s/%s", progpath, CHILD_PROGRAM_NAME);
            const char *argw[] = {CHILD_PROGRAM_NAME, "1", NULL};
            int32_t status = execv(path, argw);
            if(status == -1) {
                const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    } 
    const pid_t child2 = fork();
    if(child2 == -1) {
        const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
    } else if(child2 == 0) {
        {
			pid_t pid = getpid(); // NOTE: Get child PID

			char msg[64];
			const int32_t length = snprintf(msg, sizeof(msg),
				"%d: I'm a child\n", pid);
			write(STDOUT_FILENO, msg, length);
		}
        close(pipe1[0]);
        close(pipe1[1]);
        dup2(pipe3[0], STDIN_FILENO);
        close(pipe3[0]);
        close(pipe3[1]);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[0]);
        close(pipe2[1]);
        {
            char path[1024];
            snprintf(path, sizeof(path) - 1, "%s/%s", progpath, CHILD_PROGRAM_NAME);
            const char *argw[] = {CHILD_PROGRAM_NAME, "0", NULL};
            int32_t status = execv(path, argw);
            if(status == -1) {
                const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    } 
    // client
    pid_t pid = getpid(); 
    char msg[64];
    const int32_t length = snprintf(msg, sizeof(msg),
        "%d: I'm a Parent\n", pid);
    write(STDOUT_FILENO, msg, length);
    close(pipe3[0]);
    close(pipe3[1]);
    close(pipe2[1]);
    close(pipe1[0]);
    char buf[4096];
    ssize_t sz;
    while(sz = read(STDIN_FILENO, buf, sizeof(buf))) {
        if(sz < 0) {
            const char msg[] = "error: failed to read from stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        write(pipe1[1], buf, sz);
        sz = read(pipe2[0], buf, sizeof(buf)); // ?
        write(STDOUT_FILENO, buf, sz);
    }
    close(pipe1[1]);
    close(pipe2[0]);
    wait(NULL);
    return 0;
}
