#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, const char **argv) {
    bool first = argv[1][0] - '0';
    char buf[4096];
    ssize_t sz;
    while(sz = read(STDIN_FILENO, buf, sizeof(buf))) {
        if(sz < 0) {
            const char msg[] = "error: failed to read from stdin\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        // преобразовать buf
        if(first) { // child 1
            for(size_t i = 0; i < sz; ++i) buf[i] = tolower(buf[i]);
            write(STDOUT_FILENO, buf, sz);
        } else { // child 2
            char nbuf[4096];
            size_t nsz = 0;
            for(size_t i = 0; i < sz; ++i) {
                if(buf[i] != ' ' || i == sz - 1 || buf[i + 1] != ' ') {
                    nbuf[nsz] = buf[i];
                    nsz++;
                }
            }
            write(STDOUT_FILENO, nbuf, nsz);
        }
    }
    return 0;
}