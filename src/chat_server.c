


#include<stdio.h>
#include<sys/epoll.h>
#include <stdlib.h>

#include "network_s.h"
#include "config.h"
#include "epoll_s.h"
#include "ChatSystem.h"
#include"threadpool.h"

// 接收消息的池子
RecvBuf* recvPool[500];
ThreadPool* pool;
SendBuf* sendPool[500];
void cleanup() {
	
	threadExit(pool);

}
int main(){

	int listen_fd=init_listen(SERVER_IP, SERVER_PORT);

	int epoll_fd=create_epoll(listen_fd);
	memset(recvPool,0,sizeof(recvPool));
	memset(sendPool,0,sizeof(sendPool));

	pool=threadPoolCreate(3,20,600);

    atexit(cleanup);

	while (1)
	{
		// 处理时间
		printf("处理！！\n");
		handle_events(epoll_fd,listen_fd);
	}
	
}