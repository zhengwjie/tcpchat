#pragma once


typedef struct data
{
	int len;
	int event_type;
	char buffer[1024];
	/* data */
};

int initialize_socket(const char *server_address, int port);
int send_data(int socket, const void *data, int size);
int receive_data(int socket, void *buffer, int buffer_size);
