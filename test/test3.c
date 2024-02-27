#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

void getCursorPosition(int *row, int *col) {
	struct termios term, term_orig;
    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return -1;
    term_orig = term;
    term.c_lflag &= ~(ECHO);
    // term.c_cc[VMIN] = 1;
    // term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
        return -1;
    // 发送 ANSI Escape Code 获取光标位置
    printf("\033[6n");

    // 读取终端的回应，解析得到行和列
    scanf("\033[%d;%dR", row, col);

	tcsetattr(STDIN_FILENO,TCSANOW,&term_orig);



}

int main() {
    int row, col;

    // 获取光标当前位置
    getCursorPosition(&row, &col);

    // 输出当前光标位置
    printf("Current cursor position: Row %d, Column %d\n", row, col);

    return 0;
}
