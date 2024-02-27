#pragma once

#include <unistd.h>

// 初始化
// 处理请求
// 管理数据
// 

/// @brief 初始化监听socket
/// @param server_ip 服务器的ip地址 SERVER_IP
/// @param port 服务器的端口号 78
/// @return 返回监听的socket
int init_listen(const char* server_ip, int port);

void handle_new_connection(int server_socket, int epoll_fd);

void handle_client_data(int client_socket);

