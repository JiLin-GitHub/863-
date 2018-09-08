#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
int main(int argc, char *argv[])
{
	unsigned short port = 8080;        		// 服务器的端口号
	char *server_ip = "10.221.20.24";    	// 服务器ip地址
	
	if( argc > 1 )		//函数传参，可以更改服务器的ip地址									
	{		
		server_ip = argv[1];
	}	
	if( argc > 2 )	   //函数传参，可以更改服务器的端口号									
	{
		port = atoi(argv[2]);
	}

	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);// 创建通信端点：套接字
	if(sockfd < 0)
	{
		perror("socket");
		exit(-1);
	}
	
	// 设置服务器地址结构体
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr)); // 初始化服务器地址
	server_addr.sin_family = AF_INET;	// IPv4
	server_addr.sin_port = htons(port);	// 端口
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);	// ip
	
	 // 主动连接服务器
	int err_log = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));     
	if(err_log != 0)
	{
		perror("connect");
		close(sockfd);
		exit(-1);
	}
	
	char send_buf[512] = {0};
	printf("send data to %s:%d\n",server_ip,port);
	while(1)
	{
		printf("send:");
		fgets(send_buf,sizeof(send_buf),stdin); // 输入内容
		send_buf[strlen(send_buf)-1]='\0';
		send(sockfd, send_buf, strlen(send_buf), 0);   // 向服务器发送信息
	}

	close(sockfd);
	
	return 0;
}