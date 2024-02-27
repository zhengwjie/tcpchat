
#include<stdio.h>
#include<string.h>
#include <wchar.h>
#include <locale.h>
#include<termio.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include<pthread.h>

// #include <ncurses.h>

#include "config.h"
#include "network.h"
#include "common.h"
#include "ChatSystem.h"
static int row,col;

int connfd=-1;

// 连接到服务器
// 开始发送数据

// 网络通信
// 数据的发送和接收
void init();
#define ANSI_MOVE_CURSOR "\033[%d;%dH"
pthread_mutex_t win_lock;

void input_passwd(char* passwd){
	printf("please input passwd: ");
	char ch;
	int i=0;
	struct termios original_term, modified_term;
    tcgetattr(STDIN_FILENO, &original_term);

    modified_term = original_term;
    modified_term.c_lflag &= ~ECHO; // 关闭回显

    tcsetattr(STDIN_FILENO, TCSANOW, &modified_term);

    while ((ch=getchar())!='\n')
	{
		/* code */
		passwd[i++]=ch;
		putchar('*');
	}
	passwd[i]='\0';
	tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
	printf("\n");
	fflush(stdout);
}
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // 读取并丢弃输入缓冲区中的字符，直到遇到换行符或文件结束符
    }
}

void input_name(char* name){
	printf("please input user name: ");
	scanf("%s",name);
}

void sign_up(){
    char user_name[25];
	char passwd[25];
	// 通信
	// 注册成功
	input_name(user_name);
	clearInputBuffer();
    input_passwd(passwd);
	// scanf("%s",passwd);
	printf("\nyour passwd: %s\n",passwd);
    // TO DO:
	// 要发送注册信息给服务器

	printf("Congratulations! Login successful!\n");
	init();
}
void printline(char ch){
	// printf("col:: %d\n",col);
	for(int i=0;i<col;++i){
		printf("%c",ch);
	}
	// printf("\n");
}
// 向显示的终端输出信息
void printMsg(const char* str){
	// 记录这个视图中的光标的位置
	int cur_row,cur_col;
	get_cur_position(&cur_col,&cur_row);
	static int view_row,view_col;
	static int initialized=0;
	if(!initialized){
		view_row=cur_row,view_col=cur_col;
		initialized=1;
	}
	pthread_mutex_lock(&win_lock);
	// printf("row: %d, col: %d\n",view_row,view_col);
    // 修改光标位置
	printf("\033[%d;%dH",view_row,view_col);
	fflush(stdout);

	printf("%s",str);
	// 获取当前的位置
	get_cur_position(&cur_col,&cur_row);
	// printf("cur row: %d, cur col: %d",cur_row,cur_col);
	view_row=cur_row;
	view_col=cur_col;
	// printf("row: %d, col: %d",view_row,view_col);
	if(view_row>=row-2){
		printline(' ');
		printf("\n");
		printline(' ');
		printf("\n\n");
		fflush(stdout);
		view_row=row-3;
		view_col=1;
	}
	printf("\033[%d;%dH",row-2,1);
	fflush(stdout);
	printline('-');
	// 移动光标，然后输入----- 以及>
	pthread_mutex_unlock(&win_lock);
}

void clearline(){
	int cur_row,cur_col;
	get_cur_position(&cur_col,&cur_row);
	// printf("row: %d, col: %d\n",cur_row,cur_col);
	pthread_mutex_lock(&win_lock);
	
	printf("\033[%d;%dH",cur_row-1,1);
	// char buffer[20]; // Adjust the buffer size as needed

    // // Format the string
    // int len = sprintf(buffer, "\033[%d;%dH\n", cur_row - 1, 1);

    // // Use the write system call to write to the terminal
    // write(STDOUT_FILENO, buffer, len);
	// write(STDOUT_FILENO, "\033[6n", 4)
	printline(' ');

	pthread_mutex_unlock(&win_lock);

}
// 有一个接收线程，在后台接受数据
// 编辑区，
// 不停等待用户的输入
void edit(){
	char str[1024];
	int i=0;
	char ch;
	while (1)
	{
		printf("\033[%d;%dH>",row-1,1);
		i=0;
		while ((ch=getchar())!='\n')
		{
			/* code */
			str[i++]=ch;
		}
		if(i==0){
			continue;
		}
		str[i++]='\n';
		str[i]='\0';
		clearline();
		// 清除当前行
		// 根据用户的输出发送数据

		printMsg(str);

	}
}
void login_in(){
	char user_name[25];
	char passwd[25];
	input_name(user_name);
	clearInputBuffer();
    input_passwd(passwd);

    ClientVerifyMessage message;
    strcpy(message.user_name,user_name);
	strcpy(message.passwd,passwd);
	// 类型是32位的数据

	uint32_t type=(uint32_t)LOGIN_REQUEST;
	int pkg_len=sizeof(message)+sizeof(type);
	char* data=malloc(pkg_len+sizeof(pkg_len));
	int net_pkg_len=htonl(pkg_len);
	uint32_t net_type=htonl(type);
	memcpy(data,&net_pkg_len,sizeof(net_pkg_len));
	memcpy(data+sizeof(net_pkg_len),&net_type,sizeof(net_type));
	memcpy(data+sizeof(net_pkg_len)+sizeof(net_type),&message,sizeof(message));

    int send_size=send_data(connfd,data,pkg_len+sizeof(pkg_len));
	printf("send %d bytes.\n",send_size);
	
	// 进行通信
	// system("clear");
	// 通信模块
	printMsg("Congratulations! Login successful!\n");
	// printMsg("hhh\n");
	// printMsg("ggggh\n");
	edit();
	// printMsg("hhh\n");
	// printf("\n恭喜你！！！登录成功！！！\n");
	// 展示聊天列表
	// 并且，可以发送消息
	// get_term_size(&row,&col);
	// printf("row: %d, col: %d \n",row,col);
}

void welcome(){
	printf("Welcom to Chat System!!!\n");
	int res=get_term_size(&row,&col);
    printf("res: %d\n",res);
	printf("win row: %d, win col: %d\n",row,col);
	int cur_row,cur_col;
    get_cur_position(&cur_col,&cur_row);
	printf("cur row: %d, cur col: %d\n",cur_row,cur_col);
	get_cur_position(&cur_col,&cur_row);
	printf("cur row: %d, cur col: %d\n",cur_row,cur_col);
	get_cur_position(&cur_col,&cur_row);
	printf("cur row: %d, cur col: %d\n",cur_row,cur_col);

	if(connfd==-1){
		connfd=initialize_socket(SERVER_IP, SERVER_PORT);
	}
	pthread_mutex_init(&win_lock,NULL);

	// 完成连接
	// 可以发送数据了
}

void chat_exit(){
	close(connfd);
}
void init(){
	// 提供用户两个选择
	// 1. 登录到服务器
	// 2. 注册用户
	// 3. 退出系统
    welcome();
    
	printf("Please select an operation:\n1. Log in to the server\n2. Register a user\n3. Exit the system\n");
	int opt;
	scanf("%d",&opt);
	switch (opt)
	{
	case 1:
		/* code */
		login_in();
		break;
	case 2:
	    sign_up();
	    break;
	default:
	    chat_exit();
	    printf("Exit Chat System!!!\n");
		return;
	}
}

int main(){

    // setlocale(LC_ALL, "en_US.UTF-8");
	// setlocale(LC_ALL, "");

	init();
	
	// system("cls");
	// initscr();  // 初始化ncurses
	// clear(); 
	

	// welcome();
	// init();
	// char ch[]="我是中国人";

	// for(int i=0;ch[i]!='\0';++i){
	// 	printf("%d\n",ch[i]);
	// }

}