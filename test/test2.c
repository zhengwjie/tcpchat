#include <stdio.h>

void setCursorPosition(int row, int col) {
    // 使用 ANSI Escape Codes 将光标移动到指定行列
    printf("\033[%d;%dH", row, col);
}

int main() {
    // 移动光标到第 5 行第 10 列
    setCursorPosition(5, 10);

    // 输出一些内容
    printf("Hello, Cursor!");

    // 移动光标到第 8 行第 1 列
    setCursorPosition(8, 1);

    // 输出另一些内容
    printf("New Line!");

    return 0;
}
