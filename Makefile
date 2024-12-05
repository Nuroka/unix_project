# 컴파일러 설정
CC = gcc

# 실행 파일 이름
SERVER_EXEC = server
CLIENT_EXEC = client

# 소스 파일
SERVER_SRC = server.c game_protocol.c
CLIENT_SRC = client.c game_protocol.c

# 빌드 규칙
all: $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(SERVER_SRC)
	$(CC) -o $(SERVER_EXEC) $(SERVER_SRC)

$(CLIENT_EXEC): $(CLIENT_SRC)
	$(CC) -o $(CLIENT_EXEC) $(CLIENT_SRC)

clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)
