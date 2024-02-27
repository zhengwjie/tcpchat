#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }

    printf("Terminal window size: %d rows x %d columns\n", ws.ws_row, ws.ws_col);

    return 0;
}
