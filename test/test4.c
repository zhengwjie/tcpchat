#include <stdio.h>
#include <sys/ioctl.h>

void getCursorPosition(int *row, int *col) {
    struct winsize ws;
    
    // 使用 ioctl 获取终端窗口大小
    if (ioctl(0, TIOCGWINSZ, &ws) != -1) {
        *row = ws.ws_row;
        *col = ws.ws_col;
    } else {
        perror("ioctl");
    }
}

int main() {
    int row, col;

    // 获取当前光标位置
    getCursorPosition(&row, &col);

    // 输出当前光标位置
    printf("Current cursor position: Row %d, Column %d\n", row, col);
    printf("\033[%d;%dH",1,3);
	printf("hhhhh");
    return 0;
}
