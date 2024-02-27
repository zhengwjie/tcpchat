#include <stdio.h>

void setCursorPosition(int row, int col) {
    printf("\033[%d;%dH", row, col);
	fflush(stdout);
}

int main() {
    // 使用setCursorPosition函数设置光标位置为第5行第1列
    setCursorPosition(1, 5);
	printf("hhhhh\n");

    // 其他的程序逻辑...

    return 0;
}
