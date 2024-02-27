
#include<sys/epoll.h>
#include<sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include<stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include "epoll_s.h"
#include "config.h"
#include "ChatSystem.h"
#include "threadpool.h"

int create_epoll(int listen_fd){
    // 创建对象

	int epoll_fd=epoll_create(MAX_CONNECTION);

    op_event(epoll_fd,listen_fd,EPOLLIN,EPOLL_CTL_ADD);

	return epoll_fd;
}

void op_event(int epoll_fd,int fd,int state,uint32_t op){
	struct epoll_event ev;
	ev.events=state;
	ev.data.fd=fd;
    epoll_ctl(epoll_fd,op,fd,&ev);
}
int set_nonblock(int fd){
	int flags= fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

void handle_connection(int epoll_fd,int listen_fd){

	struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    socklen_t socklen=sizeof(addr);
	
	int connect_fd=accept(listen_fd,(struct sockaddr*)&addr,&socklen);
    if(connect_fd<0){
		perror("Erro connecting");
	}else{
		// ntohs (network to host short)
		// htons (host to network short)
		// 主机字节顺序和网络字节序列之间的转换

		set_nonblock(connect_fd);
		printf("Connect to a new client: %s:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		// 使用epoll接管
		// 刚连接，使用
		op_event(epoll_fd,connect_fd,EPOLLIN,EPOLL_CTL_ADD);
	}
}

// 暂时只考虑单线程模型处理
// Reactor模型
void handle_events(int epoll_fd,int listen_fd){
    // struct 
	struct epoll_event events[MAX_EVENTS];
    // 阻塞如果没有消息就阻塞

	int ev_cnt=epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
    printf("ev_cnt: %d\n",ev_cnt);

	for(int i=0;i<ev_cnt;++i){
		handle_event(&events[i], epoll_fd,listen_fd);
	}
}

extern ThreadPool* pool;
extern RecvBuf* recvPool[500];


void handle_event(struct epoll_event* ev,int epoll_fd,int listen_fd){
	int fd=ev->data.fd;
	if(fd==listen_fd){
		handle_connection(epoll_fd,listen_fd);
	}else if(ev->events & EPOLLIN){
		//当前fd可读
		// 可以从这个fd中读取数据
		// 首先使用单线程模型读取数据

        int fd=ev->data.fd;
		// 根据fd找到目的地
        RecvBuf* cur=recvPool[fd%500];
		while (cur!=NULL&&cur->fd!=fd)
		{
			cur=cur->next;
		}
		if(cur==NULL){
			// 创建一个节点
			// 初始化节点
			cur=(RecvBuf*)malloc(sizeof(RecvBuf));
			cur->fd=fd;
			cur->next=recvPool[fd%500];
			cur->buf_len=1024;
		    cur->recved_len=0;
			cur->bigger_buf=cur->buf;
			recvPool[fd%500]=cur;
			// 读取包长
			int byte_received=recv(fd,(void*)&(cur->pack_len),sizeof(cur->pack_len),0);
			if(byte_received!=sizeof(cur->pack_len)){
				perror("received data error");
			}
			// 转换字节序
			cur->pack_len=ntohl(cur->pack_len);
			// printf("read some data\n %d \n",cur->pack_len);
			// 数据包超出1024的时候
			if(cur->pack_len>cur->buf_len){
				cur->bigger_buf=malloc(cur->pack_len);
				cur->buf_len=cur->pack_len;
			}
		}
		// 开始接收数据
        int res=read_data(cur);
		if(res==0&&cur->pack_len==cur->recved_len){
			// 解析数据
			parse_data(cur);
			clean_buf(cur);
			// delete(cur);
		}
	}else{
		// fd可写
		// 可以往这个fd中写数据
		handle_write();
	}
}

ClientVerifyMessage* parse_verify_message(char* data){
	ClientVerifyMessage* old_msg=(ClientChatMessage*)data;
	ClientVerifyMessage* message=malloc(sizeof(ClientChatMessage));
	strcpy(message->user_name,old_msg->user_name);
	strcpy(message->passwd,old_msg->passwd);
	return message;
}

int parse_data(RecvBuf* cur){
    
    uint32_t net_type;
    memcpy(&net_type, cur->bigger_buf, sizeof(net_type));
    uint32_t type = ntohl(net_type);

	// 根据type去解析数据
	if(type==LOGIN_REQUEST){
		ClientVerifyMessage* msg=parse_verify_message((cur->bigger_buf)+sizeof(net_type));
		printf("user name: %s\n",msg->user_name);
		printf("passwd: %s\n",msg->passwd);

	}else if(type==REGISTER_REQUEST){

	}

}
// 读取数据
int read_data(RecvBuf* buf_ptr){
    do{
		int bytes_recved=recv(buf_ptr->fd,
		(buf_ptr->bigger_buf)+(buf_ptr->recved_len),
		(buf_ptr->pack_len)-(buf_ptr->recved_len),0);
		if(bytes_recved<0){
			// 收不到数据了
			if(bytes_recved==-1&&(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)){
				break;
			}else{
				//用户下线了（后面再处理业务）
				// 清除这个缓存区
				close(buf_ptr->fd);
				delete(buf_ptr);
				return -1;
			}
		}
		if(bytes_recved==0){
			//用户下线了（后面再处理业务）
			// 清除这个缓存区
			close(buf_ptr->fd);
			delete(buf_ptr);
			return -1;
		}
		(buf_ptr->recved_len)+=bytes_recved;
	}while(1);
	return 0;
}

void clean_buf(RecvBuf* buf_ptr){
	buf_ptr->pack_len=0;
	buf_ptr->recved_len=0;
}
void delete(RecvBuf* buf_ptr){
	if(buf_ptr==recvPool[(buf_ptr->fd)%500]){
		recvPool[(buf_ptr->fd)%500]=buf_ptr->next;
		free(buf_ptr);
		buf_ptr=NULL;
		return;
	}
	RecvBuf* prev=NULL, *cur=recvPool[(buf_ptr->fd)%500];
	while (cur!=NULL&&cur!=buf_ptr)
	{
		prev=cur;
		cur=cur->next;
	}
	prev->next=cur->next;
	if(buf_ptr->buf!=buf_ptr->bigger_buf){
		free(buf_ptr->bigger_buf);
	}
	free(buf_ptr);
	buf_ptr=NULL;
}

// 服务器接收消息
// 1. 登录
// 2. 注册
// 3. 发消息 向某个群组发消息
// 4. 创建群组
// 5.
// 处理读取数据的逻辑

// 根据 fd 确定消息
// 看看 fd 中是否有缓存
extern RecvBuf* recvPool[500];

void handle_read(void* arg){
    // 非阻塞的方式读取
	// 在线程池中读取数据并且写入
	// 先读取数据
	// 然后再处理
	// 先找到包的长度，然后再去读
	// 处理数据
	// 处理之后再去读
	printf("开始读取数据\n");

	ChatEvent* event=(ChatEvent*)arg;
	int epoll_fd=event->epoll_fd;
	int fd=event->fd;
	// 获取缓存区
	// 
	int idx=fd%500;
	RecvBuf* buf_ptr=recvPool[idx];
	while (buf_ptr!=NULL)
	{
		if(buf_ptr->fd==fd){
			break;
		}else{
			buf_ptr=buf_ptr->next;
		}
	}
	// 说明没有这个节点，创建这一个节点
	if(buf_ptr==NULL){
		buf_ptr=malloc(sizeof(RecvBuf));
		buf_ptr->fd=fd;
		buf_ptr->recved_len=0;
		buf_ptr->buf_len=1024;
		buf_ptr->pack_len=0;
		buf_ptr->bigger_buf=buf_ptr->buf;
		buf_ptr->next=recvPool[idx];
	}
	// 存在这个节点，不用做任何操作
	// 开始接收数据
	// 最后再消费数据
	// 说明不知道刚开始接收
	if(buf_ptr->pack_len==0){
		// 4个字节
		// 读取包头长度
		// 并且赋值
		int bytes_recved=recv(buf_ptr->fd,&buf_ptr->pack_len,sizeof(int32_t),0);
		if(bytes_recved<0){
			perror("read package length error.\n");
			return -1;
		}else if(bytes_recved==0){
			perror("This connection had been closed.\n");
			return -1;
		}
	    buf_ptr->pack_len=ntohl(buf_ptr->pack_len);
		// 检查长度是否超出了1024
		// 超过的话就malloc
		printf("buf len: %d\n",buf_ptr->pack_len);
		if(buf_ptr->pack_len>buf_ptr->buf_len){
			// 超出缓存的话，直接分配一块更大的内存。
			buf_ptr->buf_len=buf_ptr->pack_len;
			buf_ptr->bigger_buf=malloc(buf_ptr->buf_len);
			// 暂时不考虑大文件
		}
	}
	// 使用非阻塞方式读取数据
	do{
		int bytes_recved=recv(buf_ptr->fd,
		buf_ptr->bigger_buf+buf_ptr->recved_len,
		buf_ptr->pack_len-buf_ptr->recved_len,0);
		if(bytes_recved<=0){
			// 收不到数据了
			if(bytes_recved==0||(bytes_recved==-1&&errno == EAGAIN)){
				break;
			}else{
				// 
				close(buf_ptr->fd);
				return -1;
			}
		}
	}while(1);
	// 包的长度等于消息长度
	// 检查是否可以读取完成
	if(buf_ptr->pack_len==buf_ptr->recved_len){
		// 读取完成
		// 开始解析
		// 解析完成之后，
		// 转发消息
		// buf_ptr->bigger_buf 解析这个就可以了。

        printf("在这里收到一个数据包！！！");

	}
	// parse the package.
	// 如果读取完成，拆解包的信息

}

// 服务器发送消息
void handle_write(){

}