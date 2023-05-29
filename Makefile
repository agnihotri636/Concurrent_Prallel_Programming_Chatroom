# Compiler and flags
CC = g++
FLAGS = -std=c++11 -Wall -pthread

all: chat_server chat_client

chat_server: chat_server.o
	$(CC) $(FLAGS) -o chat_server chat_server.o

chat_client: chat_client.o
	$(CC) $(FLAGS) -o chat_client chat_client.o


chat_server.o: chat_server.cpp
	$(CC) $(FLAGS) -c chat_server.cpp

chat_client.o: chat_client.cpp
	$(CC) $(FLAGS) -c chat_client.cpp


clean:
	rm -f chat_server chat_client *.o

