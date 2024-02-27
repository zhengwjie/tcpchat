#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


int initialize_socket(const char *server_address, int port){
	 // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        return -1;
    }

    // 设置服务器地址和端口
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_address, &(server.sin_addr)) <= 0) {
        perror("Error converting server address");
        close(client_socket);
        return -1;
    }
    // 连接到服务器
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        return -1;
    }
    return client_socket;
}

// 一直发数据，直到发送完成
int send_data(int socket, const void *data, int size){
	if (socket < 0) {
        perror("Invalid socket descriptor\n");
        return -1;
    }
    if (data == NULL || size <= 0) {
        perror("Invalid data or size\n");
        return -1;
    }
	int bytes_sent=0;
	do{
		size_t sent=send(socket, data+bytes_sent, size-bytes_sent, 0);
		if (sent == -1) {
            perror("Error sending data");
            return -1;
		}
		bytes_sent+=sent;
	}while(bytes_sent<size);

    // 返回实际发送的字节数
    return bytes_sent;
}

int receive_data(int socket, void *buffer, int buffer_size){
	
	if (socket < 0) {
        fprintf(stderr, "Invalid socket descriptor\n");
        return -1;
    }

    if (buffer == NULL || buffer_size <= 0) {
        fprintf(stderr, "Invalid buffer or buffer size\n");
        return -1;
    }

    // 使用 recv 函数接收数据
    ssize_t bytes_received = recv(socket, buffer, buffer_size, 0);

    if (bytes_received == -1) {
        perror("Error receiving data");
        return -1;
    }
    if (bytes_received == 0) {
        // 连接已关闭
        fprintf(stderr, "Connection closed by peer\n");
        return 0;
    }
    // 返回实际接收的字节数
    return bytes_received;
}
