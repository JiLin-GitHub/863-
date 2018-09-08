###########################################  
#Makefile for simple programs  
###########################################  

CC = /home/openwrt/7620/openwrt/barrier_breaker/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc
#CC = gcc
TARGET = opcua
OBJS = opcua_str_prase_new.o open62541.o
#CFLAGS = -std=c99 -D_REENTRANT -DDEBUG -DDEBUG_PRINT -g -Wall 
CFLAGS = -D_REENTRANT -DDEBUG -g -Wall -std=c99 -lrt -rdynamic
RM = rm -f

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -lpthread

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET) $(OBJS)

 
#LIB= -lpthread 
# 
##CC=mipsel-openwrt-linux-gcc
#CC=gcc  
#CC_FLAG=-g -Wall  
# 
#PRG=opcua_str_prase_new  
#OBJ=opcua_str_prase_new.o
#
#$(PRG):$(OBJ)
#	@echo "正在编译执行文件。"  
#	$(CC) -o $@ $< $(LIB) 
#$(OBJ):%.o:%.c 
#	@echo "正在编译.o文件。" 
#	$(CC) $(CC_FLAG) -c $<
# 
#.PRONY:clean
# 
#clean:
#	@echo "正在删除编译文件。"   
#	rm -f $(OBJ) $(PRG)

	
	
