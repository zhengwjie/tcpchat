#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

void setTerminalSize(int rows, int cols) {
    struct winsize ws;
    ws.ws_row = rows;
    ws.ws_col = cols;

    if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1) {
        perror("ioctl");
    }
}

int main() {
    // 设置终端窗口大小为 30 行 x 80 列
    setTerminalSize(30, 80);

    printf("Terminal window size has been set to 30 rows x 80 columns.\n");

    return 0;
}
