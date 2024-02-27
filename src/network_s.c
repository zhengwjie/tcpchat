#include "network_s.h"
#include <arpa/inet.h>

/// @brief 初始化监听socket
/// @param server_ip 服务器的ip地址 SERVER_IP
/// @param port 服务器的端口号 78
/// @return 返回监听的socket
int init_listen(const char* server_ip, int port){
	int fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){
		perror("Error creating socket");
		return -1;
	}
	struct sockaddr_in addr;
	
    addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	// s_addr就是32位的一个数
	if(inet_pton(AF_INET,server_ip,&addr.sin_addr)<=0){
		perror("Error converting Ip address");
		close(fd);
		return -2;
	}
	int opt = 1;
    setsockopt( fd, SOL_SOCKET,SO_REUSEADDR,(const void *)&opt, sizeof(opt) );
	if(bind(fd,&addr,sizeof(addr))<0){
		perror("Error binding Ip address");
		close(fd);
		return -3;
	}
	
	// 标记套接字为被动套接字，等待连接请求
    if (listen(fd, 5) == -1) {
        perror("Error listening on socket");
        close(fd);
        return -4;
    }
    printf("Server is listening for incoming connections\n");
	return fd;
}

void handle_new_connection(int server_socket, int epoll_fd) {
    // struct sockaddr_in client_addr;
    // socklen_t client_addr_len = sizeof(client_addr);
    // int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    // if (client_socket == -1) {
    //     perror("Accept failed");
    //     return;
    // }

    // // Add the new client socket to epoll
    // struct epoll_event event;
    // event.events = EPOLLIN;
    // event.data.fd = client_socket;
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event);

    // printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

void handle_client_data(int client_socket) {
    // Implement logic to handle data from the client
    // Use chat_system to manage users and messages

    // For example:
    // Receive data from the client using recv()
    // Process the received data, update chat_system, send responses, etc.
}
