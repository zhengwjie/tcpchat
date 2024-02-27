#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define MAX_PASSWORD_LENGTH 20

void getPassword(char *password) {
    int i = 0;
    char ch;

    printf("请输入密码： ");

    struct termios original_term, modified_term;
    tcgetattr(STDIN_FILENO, &original_term);

    modified_term = original_term;
    modified_term.c_lflag &= ~ECHO; // 关闭回显

    tcsetattr(STDIN_FILENO, TCSANOW, &modified_term);

    while ((ch = getchar()) != '\n' && i < MAX_PASSWORD_LENGTH - 1) {
        password[i++] = ch;
        putchar('*'); // 输出*替代实际字符
    }

    password[i] = '\0'; // 在字符串末尾添加null字符

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

int main() {
    char password[MAX_PASSWORD_LENGTH];
    getPassword(password);

    printf("\n你输入的密码是：%s\n", password);
	
    return 0;
}
