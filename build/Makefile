
CC=arm-linux-gnueabihf-gcc
INCLUDE=-I../include
client: ../src/chat_client.c ../src/network.c ../src/common.c
	$(CC) $(INCLUDE) -o client ../src/chat_client.c ../src/network.c ../src/common.c
	chmod +x $@

server: ../src/chat_server.c
	$(CC) $(INCLUDE) -o server ../src/chat_server.c
	chmod +x $@

client_cur: ../src/chat_client.c ../src/network.c ../src/common.c
	gcc $(INCLUDE) -o client_cur ../src/chat_client.c ../src/network.c ../src/common.c
	chmod +x $@

SERVER_SOURCES=../src/chat_server.c ../src/epoll_s.c ../src/network_s.c ../src/threadpool.c
server_cur: $(SERVER_SOURCES)
	gcc $(INCLUDE) -o server_cur $(SERVER_SOURCES) -lpthread
	chmod +x $@
	

test: ../src/test.c
	gcc $(INCLUDE) -o test ../src/test.c
	chmod +x $@

%: ../test/%.c
	$(CC) $(INCLUDE) -o $@ ../test/$@.c
	chmod +x $@

.PHONY: clean
clean:
	rm client* server* 