OBJ += tcp_recv.o
FLAGS = -Wall
CC = gcc
TARGET = tcp_recv
$(TARGET):$(OBJ)  
	$(CC) $(OBJ) -o $@ $(FLAGS)
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS)
.PHONY:clean
clean:
	rm example *.o $(TARGET) -rfv	
