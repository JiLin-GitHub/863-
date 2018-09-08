OBJ += tcp_send.o
FLAGS = -Wall
CC = gcc
TARGET = tcp_send
$(TARGET):$(OBJ)  
	$(CC) $(OBJ) -o $@ $(FLAGS)
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS)
.PHONY:clean
clean:
	rm example *.o $(TARGET) -rfv	
