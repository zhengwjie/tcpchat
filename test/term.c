#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int get_cur_position(int *x, int *y) {
    struct termios term, term_orig;
    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return -1;
    term_orig = term;
    term.c_lflag &= ~(ICANON|ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
        return -1;
    if (write(STDOUT_FILENO, "\033[6n", 4) != 4)
        return -1;
    char buf[32] = {0};
    int i = 0;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    if (i == sizeof(buf) - 1)
        return -1;
    if (buf[0] != '\033' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", y, x) != 2)
        return -1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_orig) == -1)
        return -1;
    return 0;
}

int main() {
    int x, y;
    if (get_cur_position(&x, &y) == 0)
        printf("The caret position is (%d, %d)", x, y);
    else
        printf("Failed to get the caret position.\n");
    return 0;
}