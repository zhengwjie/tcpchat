#include <stdio.h>
#include <sys/ioctl.h>

void print(){
	for(int i=0;i<500;++i){
		printf("a");
	}
	printf("\n");
}

int getterm_size(int * row,int* col){
	// 获取当前终端窗口大小
    if (ioctl(0, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }
	*row=ws.ws_row;
	*col=ws.ws_col;
}
int main() {
    struct winsize ws;

    // 获取当前终端窗口大小
    if (ioctl(0, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }
    printf("Current window size: %d rows x %d columns\n", ws.ws_row, ws.ws_col);
    print();
	// fflush();
    // 设置新的终端窗口大小
    ws.ws_row = 20; // 30行
    ws.ws_col = 20; // 80列
    if (ioctl(0, TIOCSWINSZ, &ws) == -1) {
        perror("ioctl");
        return 1;
    }
	print();
	// fflush();
    printf("New window size: %d rows x %d columns\n", ws.ws_row, ws.ws_col);

    sleep(10);
	

    return 0;
}
