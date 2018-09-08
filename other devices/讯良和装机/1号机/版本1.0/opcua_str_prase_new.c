#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/tcp.h>
#include <fcntl.h> 	//ÎÄ¼þ¿ØÖÆ¶¨Òå
#include <termios.h>	//POSIXÖÐ¶Ï¿ØÖÆ¶¨Ò
#include <errno.h>	//´íÎóºÅ¶¨Òå


#include "open62541.h"

#define	DEBUG_DATA
#define serial_device "/dev/ttyS0"
#define STRING_LEN 30
#define OPCUA_SERVER_PORT 16664
#define SERIAL_232_PRESET_REQ_LENGTH  4
#define SLAVE 1

typedef struct _DATA_SOURCE {
	char* name;
	unsigned char type;   // 1 stand for string, 2 stand for float.
	float data;
	char string_data[STRING_LEN];
}DATA_SOURCE;

//º¯ÊýÉêÃ÷
void OpenSreial(int *serial_fd);
int serialport();
void set_speed_and_parity(int fd);
int open_port(void);

UA_Server *g_opc_server;
#define YES 1
#define S_PORT 5888
#define R_PORT 5222
#define BACKLOG 7
int s_fd = 0;
int ret = 0;
int r_fd = 0;
int Serial_fd=0; //ÐÂ¼Ó´®¿Úfd

struct argument{
	int fd;
	int port;
	int serverfd;
};
struct argument sendOriginalDataFD;
//struct argument recvDataFD;
DATA_SOURCE source[]={
		{"boot time",1,0.0,{0}},
		{"down time",1,0.0,{0}},	
		{"fault time",1,0.0,{0}},
		{"fault",2,0.0,{0}},
		{"Production quantity",2,0.0,{0}}
		};

void debug_data_source()
{
	int i;	
	printf("variable----Type----Data\n");
	for(i=0;i<sizeof(source)/sizeof(DATA_SOURCE);i++) {
		if(source[i].type == 1)
			printf("%s     %d      %s\n", source[i].name,source[i].type,source[i].string_data);
		else if(source[i].type == 2)
			printf("%s     %d      %f\n", source[i].name,source[i].type,source[i].data); 			
	}
}

int sum_check_pre(unsigned char *buffer, unsigned int length)
{
    unsigned int crc=0xffff;
    unsigned int i; /* will index into CRC lookup */
    unsigned char a,b,c;
    unsigned int buffer_length=length-2;
		
		crc = sumcheck(buffer,buffer_length);
    return crc;
}



int UartDataCheck(unsigned char *buf,int len)
{
	  unsigned int crc_late=0xffff;
	  int len_1=len-1;
	  unsigned int crc_forward = (buf[len_1]<<8) + buf[len_1-1];
	  crc_late=(unsigned int)sum_check_pre(buf,len);
	  if(crc_late == crc_forward)
	  	{return 1;}
	  else if(crc_late != crc_forward)
	  	{return -1;}
}

int praseStrToData(unsigned char *str,int length)
{
		if(length < 1)
		{
			return -1;	
		}
	if(UartDataCheck(str,length) == 1)
		{	
			printf("data check success!!!\n");
			unsigned char boottime[7]={0};
			unsigned char downtime[7]={0};
			unsigned char faulttime[7]={0};
			
			unsigned char boottime_1[15]={0};
			unsigned char downtime_1[15]={0};
			unsigned char faulttime_1[15]={0};
			unsigned char Nofault[14]="--------------";
			unsigned int a;
			int i,j;
			int len =7;
			unsigned char p[3]={0};
			
			boottime[0]=str[3];	//year
			boottime[1]=str[2];
			boottime[2]=str[4];	//month
			boottime[3]=str[6];	//day
			boottime[4]=str[8];	//shi
			boottime[5]=str[10];	//fen
			boottime[6]=str[12];	//miao
			
			downtime[0]=str[15];
			downtime[1]=str[14];	//nian
			downtime[2]=str[16];	//yue
			downtime[3]=str[18];	//ri
			downtime[4]=str[20];	//shi
			downtime[5]=str[22];	//fen
			downtime[6]=str[24];	//miao
			
			faulttime[0]=str[27];
			faulttime[1]=str[26];	//nian
			faulttime[2]=str[28];	//yue
			faulttime[3]=str[30];	//ri
			faulttime[4]=str[32];	//shi
			faulttime[5]=str[34];	//fen
			faulttime[6]=str[36];	//miao
			
				i=0;
				j=0;
			while(i<len)
			{
				if(boottime[i]<10)
				{
					sprintf(p,"%x",boottime[i]);
					boottime_1[j] = '0';
					boottime_1[++j] = p[0];
					j++;
					i++;
				}
				else if(boottime[i] > 9)
				{
					sprintf(p,"%x",boottime[i]);
					boottime_1[j] = p[0];
					boottime_1[++j] = p[1];
					j++;
					i++;
				}
			}
			
			i=0;
			j=0;
			while(i<len)
			{
				if(downtime[i]<10)
				{
					sprintf(p,"%x",downtime[i]);
					downtime_1[j] = '0';
					downtime_1[++j] = p[0];
					j++;
					i++;
				}
				else if(downtime[i] > 9)
				{
					sprintf(p,"%x",downtime[i]);
					downtime_1[j] = p[0];
					downtime_1[++j] = p[1];
					j++;
					i++;
				}
			}

			i=0;
			j=0;
			while(i<len)
			{
				if(faulttime[i]<10)
				{
					sprintf(p,"%x",faulttime[i]);
					faulttime_1[j] = '0';
					faulttime_1[++j] = p[0];
					j++;
					i++;
				}
				else if(faulttime[i] > 9)
				{
					sprintf(p,"%x",faulttime[i]);
					faulttime_1[j] = p[0];
					faulttime_1[++j] = p[1];
					j++;
					i++;
				}
			}					
			//·â×°opcua	
			memcpy(source[0].string_data,boottime_1,14);		//¿ª»úÊ±¼ä
			memcpy(source[1].string_data,downtime_1,14);		//Í£»úÊ±¼ä
			source[3].data = (float)((str[39]<<8)+str[38]);	//ÓÐÎÞ¹ÊÕÏ
			source[4].data = (float)((str[43]<<24)+(str[42]<<16)+(str[41]<<8)+str[40]); //Éú²úÊýÁ¿
			
			
			if(source[3].data == 1)
				{
					memcpy(source[2].string_data,faulttime_1,14); 	//¹ÊÕÏ·¢ÉúÊ±¼ä
				}
			else if(source[3].data == 0)
				{
					strncpy(source[2].string_data,Nofault,14); 	//¹ÊÕÏ·¢ÉúÊ±¼ä
				}
				
				
				
			#ifdef	DEBUG_DATA
				debug_data_source();
			#endif
	  }
	  else 
	  	printf("data check error!!!\n");
	return 1;
}



void creat_server_sockfd4(int *sockfd, struct sockaddr_in *local, int portnum){
	int err;
	int optval = YES;
	int nodelay = YES;

	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sockfd < 0){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	err = setsockopt(*sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	if(err){
		perror("setsockopt");
	}
	err = setsockopt(*sockfd,IPPROTO_TCP,TCP_NODELAY,&nodelay,sizeof(nodelay));
	if(err){
		perror("setsockopt");
	}


	memset(local, 0, sizeof(struct sockaddr_in));
	local->sin_family = AF_INET;
	local->sin_addr.s_addr = htonl(INADDR_ANY);
	local->sin_port = htons(portnum);

	err = bind(*sockfd, (struct sockaddr*)local, sizeof(struct sockaddr_in));
	if(err < 0){
		perror("bind");
		exit(EXIT_FAILURE);
	}
	err = listen(*sockfd, BACKLOG);
	if(err < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

}
//´´½¨·þÎñÆ÷
void creatserver(struct argument *p){
	char addrstr[100];
	int serverfd;
	struct sockaddr_in local_addr_s;
	struct sockaddr_in from;
	unsigned int len = sizeof(from);

	creat_server_sockfd4(&serverfd,&local_addr_s,p->port);

	while(1)
	{
		p->fd = accept(serverfd, (struct sockaddr*)&from, &len);
		if(ret == -1){
			perror("accept");
			exit(EXIT_FAILURE);
		}
		struct timeval time;
		gettimeofday(&time, NULL);
		printf("time:%lds, %ldus\n",time.tv_sec,time.tv_usec);
		printf("a IPv4 client from:%s\n",inet_ntop(AF_INET, &(from.sin_addr), addrstr, INET_ADDRSTRLEN));
	}

}

void OpenSreial(int *serial_fd){
	*serial_fd=serialport();	//°üº¬´ò¿ª´®¿Ú¼°´®¿Ú»ù±¾ÉèÖÃ(²¨ÌØÂÊºÍÊý¾ÝÎ»¡¢Í£Ö¹Î»¡¢ÓÐÎÞÐ£ÑéµÈ)
	if(*serial_fd==-1){
	perror("Can not open Serial_Port 1/n£¡");
		}
	else
	printf("open&set Serial_port 1 success!!!\n");
	while(1)
		{
		sleep(10);
	}
}

int serialport()
{
	int fd;	
	//´ò¿ª´®¿Ú
	if((fd=open_port())<0)
    	{
        	perror("open_port error!!!\n");
        	return -1;
    	}
	//ÉèÖÃ²¨ÌØÂÊºÍÐ£ÑéÎ»
	set_speed_and_parity(fd);
	return (fd);
}

int open_port(void)
{
	int fd;		//´®¿ÚµÄ±êÊ¶·û
	fd=open(serial_device,O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd == -1)
	{
		//²»ÄÜ´ò¿ª´®¿Ú
		perror("open_port: Unable to open /dev/ttyS0!!!\n");
		return(fd);
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		printf("open ttys0 .....\n");
		return(fd);
	}
}

void set_speed_and_parity(int fd)
{
	struct termios Opt;	//¶¨Òåtermios½á¹¹
	if(tcgetattr(fd,&Opt)!=0)
	{
		perror("tcgetattr fd");
		return;
	}
	tcflush(fd, TCIOFLUSH);
	cfsetispeed(&Opt, B9600);		//ÉèÖÃ´®¿ÚÊäÈë²¨ÌØÂÊ
	cfsetospeed(&Opt, B9600);		//ÉèÖÃ´®¿ÚÊä³ö²¨ÌØÂÊ
	if(tcsetattr(fd, TCSANOW, &Opt) != 0)	//±£´æ²âÊÔÏÖÓÐ´®¿Ú²ÎÊýÉèÖÃ£¬ÔÚÕâÀïÈç¹û´®¿ÚºÅ³ö´í£¬»áÓÐÏà¹ØµÄ³ö´íÐÅÏ¢
	{	
		perror("tcsetattr fd");
		return;
	}
	tcflush(fd, TCIOFLUSH);
	
 	//ÉèÖÃÆæÅ¼Ð£Ñé¡ª¡ªÄ¬ÈÏ8¸öÊý¾ÝÎ»¡¢Ã»ÓÐÐ£ÑéÎ»       
    	Opt.c_cflag &= ~PARENB;		//Çå³ýÐ£ÑéÎ»
   	  Opt.c_cflag &= ~CSTOPB;		//1¸öÍ£Ö¹Î»
    	Opt.c_cflag &= ~CSIZE;		//Bit mask for data bits
    	Opt.c_cflag |= CS8;				//8¸öÊý¾ÝÎ»
	
	//ÆäËûµÄÒ»Ð©ÅäÖÃ
		Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);			//Ô­Ê¼ÊäÈë£¬ÊäÈë×Ö·ûÖ»ÊÇ±»Ô­·â²»¶¯µÄ½ÓÊÕ
		Opt.c_iflag &= ~(IXON | IXOFF | IXANY);							//Èí¼þÁ÷¿ØÖÆÎÞÐ§£¬ÒòÎªÓ²¼þÃ»ÓÐÓ²¼þÁ÷¿ØÖÆ£¬ËùÒÔ¾Í²»ÐèÒª¹ÜÁË
		//Opt.c_oflag |= ~OPOST;											//Ô­Ê¼Êä³ö·½Ê½¿ÉÒÔÍ¨¹ýÔÚc_oflagÖÐÖØÖÃOPOSTÑ¡ÏîÀ´Ñ¡Ôñ£º
		Opt.c_oflag &= ~OPOST; //Ñ¡ÔñÔ­Ê¼Êä³ö
//    	Opt.c_cflag &= ~INPCK;
   	Opt.c_cflag |= (CLOCAL | CREAD);
   	Opt.c_oflag &= ~(ONLCR | OCRNL);    
   	Opt.c_iflag &= ~(ICRNL | INLCR | IGNCR);    

	//VMIN¿ÉÒÔÖ¸¶¨¶ÁÈ¡µÄ×îÐ¡×Ö·ûÊý¡£Èç¹ûËü±»ÉèÖÃÎª0£¬ÄÇÃ´VTIMEÖµÔò»áÖ¸¶¨Ã¿¸ö×Ö·û¶ÁÈ¡µÄµÈ´ýÊ±¼ä¡£
    	Opt.c_cc[VTIME] = 1;	//ÉèÖÃ³¬Ê±Îª0sec
    	Opt.c_cc[VMIN] = 0;		//Update the Opt and do it now
    
    	//tcflush(fd, TCIOFLUSH);//´¦ÀíÎ´½ÓÊÕ×Ö·û
    	tcflush(fd, TCIFLUSH);//Òç³öÊý¾Ý¿ÉÒÔ½ÓÊÕ£¬µ«²»¶Á
    if(tcsetattr(fd, TCSANOW, &Opt) != 0)	//¼¤»îÐÂÅäÖÃ
	{	
		perror("com set error");
		return;
	}
	printf("set done!\n");
    	

}

/**
  *´®¿Ú·¢ËÍÊý¾Ýº¯Êý
  *fd£º´®¿ÚÃèÊö·û
  *data£º´ý·¢ËÍÊý¾Ý
  *datalen£ºÊý¾Ý³¤¶È
  */
int serial_write(int fd ,char *data, int datalen)
{
	int len=0;
	//»ñÈ¡Êµ¼Ê´«ÊäÊý¾ÝµÄ³¤¶È
	len=write(fd,data,datalen);
	printf("send data OK! datalen=%d\n",len);
	return len;	
}

/* Builds a RTU request header */
static int serial_232_build_request_basis( int slave, int function, int nb,unsigned char *req )
{
    req[0] = slave;
    req[1] = function;
    req[2] = nb >> 8;
    req[3] = nb & 0x00ff;

    return SERIAL_232_PRESET_REQ_LENGTH;
}

int serial_232_send_msg_pre(unsigned char *req, int req_length)
{
    unsigned int crc = (unsigned int)sumcheck(req, req_length);
    req[req_length++] = crc >> 8;
    req[req_length++] = crc & 0x00FF;
    return req_length;
}

int sumcheck(unsigned char *buffer, unsigned int buffer_length)
{
    unsigned int crc = 0xffff; /* high CRC byte initialized */
    unsigned int c=0;
    int i;
    for(i=0;i<buffer_length;i++)
    {
    		c += buffer[i];
    }
    crc=c;
    return crc;
}
 
//´Ó´®¿Ú¶ÁÈ¡Êý¾Ý
int ReadData(int fd,unsigned char *p){
	char c;
	int ret = 0;
	int num = 0;

	ret = read(fd, &c, 1);
	if(ret < 0){
		perror("recv");
		return 0;
	}
	else if(ret == 0){
		printf("ÎÞÊý¾Ý½ÓÊÕ\n");
		
		return -1;
	}
	else{
		
			*p = c;
			num = 1;
			while(1){
				ret = read(fd, p + num, 1);
					num++;
				
				if(num == 46)						//
					return num;
			}	
	}
	
}

int SocketConnected(int sock) 
{ 
	if(sock<=0) 
		return 0; 
	struct tcp_info info; 
	int len=sizeof(info); 
	getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
	//if((info.tcpi_state==TCP_ESTABLISHED))	{
	if((info.tcpi_state==1))	{
//		printf("socket connected\n"); 
		return 1; 
	} 
	else 	{ 
//		printf("socket disconnected\n"); 
		return 0; 
	} 
}
void* soureDataPrase(void *arg)
{
	int ret_send = 0;
	int num = 0;
	int iRec=0;
	int i;
  unsigned char *p;
	sleep(2);
//	printf("opcua adapter start, reviece port %d\n",R_PORT);
	printf("opcua adapter serial data start!!!\n");
	while(1){
		
				unsigned char *buf;
				int length;
				buf=(unsigned char*)malloc(sizeof(unsigned char)*20);
				memset(buf,0,20);
				length=serial_232_build_request_basis(SLAVE,4,10,buf);
				length=serial_232_send_msg_pre(buf,length);
				
				iRec=serial_write(Serial_fd,buf,length);
				
				if(iRec>0){
					printf("send message is:");
					for(i=0;i<length;i++)
					{
						printf("%x ",buf[i]);
					}
					printf("\n");
					p = (unsigned char *)malloc(sizeof(unsigned char) * 1000);
					memset(p,0,sizeof(p));//³õÊ¼»¯
					num = ReadData(Serial_fd, p);
					if(num > 0) {
					printf("receive %d byte\n",num);
					printf("received data is:");
					for(i=0;i<num;i++)
					{
						printf("%x ",p[i]);
					}
					printf("\n");
					praseStrToData(p,num);
					if(SocketConnected(sendOriginalDataFD.fd)) {
					//if(sendOriginalDataFD.fd > 0) {
						ret_send = send(sendOriginalDataFD.fd, p, num, 0);						
						if(ret_send < 0) {
							perror("send");
							close(sendOriginalDataFD.fd);
						}
						else if(ret_send == 0)
							printf("Á¬½Ó¶Ï¿ª\n");
						else
							printf("write sendOriginalDataFD_fd! ret_send=%d\n",ret_send);
					}
					}
					 else if (num == -1) {
					//client disconnect
					printf("serial port has not receiced data!!!\n");				
					//close(recvDataFD.fd);  //¹Ø±ÕsocketÁ¬½Ó   
		      		//Serial_fd=0;
					}
					free(p);
				}
				else if(iRec == -1)
					{printf("serial port has closed!!!\n");
						}
					free(buf);
				sleep(20);	
	}
			

}

#include <signal.h>

UA_Boolean running = true;
static void stopHandler(int sig) {    
	running = false;
}

void* nodeidFindData(const UA_NodeId nodeId) 
{
	int i;
	for(i=0;i<sizeof(source)/sizeof(DATA_SOURCE);i++) {
		if(strncmp((char*)nodeId.identifier.string.data, source[i].name, strlen(source[i].name)) == 0) {
			if(source[i].type == 1) {
				return &source[i].string_data[0];
			}
			else if(source[i].type == 2) {
				return (float*)&source[i].data;
			}
			
		}			
	}
	printf("not find:%s!\n",nodeId.identifier.string.data);
	return NULL;
}

static UA_StatusCode
readFloatDataSource(void *handle, const UA_NodeId nodeId, UA_Boolean sourceTimeStamp,
                const UA_NumericRange *range, UA_DataValue *value) {
    if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }		
	UA_Float currentFloat;

	if(nodeidFindData(nodeId) != NULL)
		currentFloat = *(UA_Float*)nodeidFindData(nodeId);
	else 
		currentFloat = -1;
	value->sourceTimestamp = UA_DateTime_now();
	value->hasSourceTimestamp = true;
    UA_Variant_setScalarCopy(&value->value, &currentFloat, &UA_TYPES[UA_TYPES_FLOAT]);
	value->hasValue = true;
	return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
readStringDataSource(void *handle, const UA_NodeId nodeId, UA_Boolean sourceTimeStamp,
                const UA_NumericRange *range, UA_DataValue *value) {
    if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }		
	UA_String tempString;
	UA_String_init(&tempString);
	
	tempString.length = strlen(nodeidFindData(nodeId));
	tempString.data = nodeidFindData(nodeId);
	value->sourceTimestamp = UA_DateTime_now();
	value->hasSourceTimestamp = true;
    UA_Variant_setScalarCopy(&value->value, &tempString, &UA_TYPES[UA_TYPES_STRING]);
	value->hasValue = true;
	return UA_STATUSCODE_GOOD;
}


void add_dataSource_to_opcServer()
{
	int i;
	for(i=0;i<sizeof(source)/sizeof(DATA_SOURCE);i++) {
		if(source[i].type == 1) {
			//printf("%s     %d      %s\n", source[i].name,source[i].type,source[i].string_data);
			UA_DataSource dateDataSource = (UA_DataSource) {.handle = NULL, .read = readStringDataSource, .write = NULL};
			UA_VariableAttributes *attr_string = UA_VariableAttributes_new();
    	UA_VariableAttributes_init(attr_string);
			
			UA_String *init_string = UA_String_new();
			UA_String_init(init_string);
				
			UA_Variant_setScalar(&attr_string->value, init_string, &UA_TYPES[UA_TYPES_STRING]);
			attr_string->description = UA_LOCALIZEDTEXT("en_US",source[i].name);
	    attr_string->displayName = UA_LOCALIZEDTEXT("en_US",source[i].name);
			attr_string->accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, source[i].name);
	    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, source[i].name);
	    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
//	    UA_Server_addVariableNode(g_opc_server, myIntegerNodeId, parentNodeId,
//	                              parentReferenceNodeId, myIntegerName,
//	                              UA_NODEID_NULL, *attr, NULL, NULL);
		  UA_Server_addDataSourceVariableNode(g_opc_server, myIntegerNodeId,parentNodeId,
	                              					parentReferenceNodeId, myIntegerName,
                                                UA_NODEID_NULL, *attr_string, dateDataSource, NULL);
		}
		else if(source[i].type == 2) {
			UA_DataSource dateDataSource = (UA_DataSource) {.handle = NULL, .read = readFloatDataSource, .write = NULL};
			UA_VariableAttributes *attr = UA_VariableAttributes_new();
    	UA_VariableAttributes_init(attr);
			UA_Float floatData = source[i].data;
			UA_Variant_setScalar(&attr->value, &floatData, &UA_TYPES[UA_TYPES_FLOAT]);
			attr->description = UA_LOCALIZEDTEXT("en_US",source[i].name);
	    attr->displayName = UA_LOCALIZEDTEXT("en_US",source[i].name);
			attr->accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, source[i].name);
	    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, source[i].name);
	    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
//	    UA_Server_addVariableNode(g_opc_server, myIntegerNodeId, parentNodeId,
//	                              parentReferenceNodeId, myIntegerName,
//	                              UA_NODEID_NULL, *attr, NULL, NULL);
		  UA_Server_addDataSourceVariableNode(g_opc_server, myIntegerNodeId,parentNodeId,
	                              					parentReferenceNodeId, myIntegerName,
                                                UA_NODEID_NULL, *attr, dateDataSource, NULL);


			
		}
			//printf("%s     %d      %f\n", source[i].name,source[i].type,source[i].data); 			
	}	
}
void handle_opcua_server(void * arg){
		//signal(SIGINT,  stopHandler);
    //signal(SIGTERM, stopHandler);

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, OPCUA_SERVER_PORT);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    g_opc_server = UA_Server_new(config);

		/* add a variable node to the address space */
    UA_VariableAttributes attr;
    UA_VariableAttributes_init(&attr);
    UA_Float myInteger = 0.42;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_FLOAT]);
    attr.description = UA_LOCALIZEDTEXT("en_US","the answer");
    attr.displayName = UA_LOCALIZEDTEXT("en_US","the answer");
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(g_opc_server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NULL, attr, NULL, NULL);

	add_dataSource_to_opcServer();
	
    UA_Server_run(g_opc_server, &running);
		
    UA_Server_delete(g_opc_server);
    nl.deleteMembers(&nl);  
}

int main(){

	pthread_t r_id;
	pthread_t source_Data_id;
	pthread_t original_data_outpit_id;
	pthread_t opcua_server_id;

	//recvDataFD.port = R_PORT;
	sendOriginalDataFD.port = S_PORT;
	pthread_create(&original_data_outpit_id,NULL,(void *)creatserver,&sendOriginalDataFD);
	pthread_create(&r_id,NULL,(void *)OpenSreial,&Serial_fd);
	pthread_create(&source_Data_id,NULL,(void *)soureDataPrase,&Serial_fd);
//	sleep(1);
	pthread_create(&opcua_server_id,NULL,(void *)handle_opcua_server,NULL);
	
	while(1) {
		sleep(1);
	}
	return 0;
}
#if 0
int main()
{
	char  *data_input = "?id:1457049893000,url:S101_151123,maccode:QB99014,productname:S101-17-HVAC,jobnumber:1,evaporesi:2.04k,innerloop:162,outerloop:52,resvalueone:12.86,resvaluetwo:12.86,resvaluethree:12.86,resvaluefour:12.86,currvalueone:12.86,currvaluetwo:12.86,currvaluethree:12.86,currvaluefour:12.86,fanendpressone:12.86,fanendpresstwo:12.86,fanendpressthree:12.86,fanendpressfour:12.86,cudeproid:S1011607140569,checkresult:OK,nowtime:2016-7-14 11:49:25!";
	praseStrToData(data_input);
	printf("Hello World!\n");
    return 0;
}
#endif
