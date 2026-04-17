/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2020-04-25 17:50:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "platform_net_socket.h"
#include "platform_esp8266_AT.h"
#include "string.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
#define JAP_SSID			"8B116"
#define JAP_passwoed	"8B116369852"


int platform_net_socket_connect(const char *host, const char *port, int proto)
{
	char cmd[50]={0};
	
	volatile int ret;
	while(1)
	{
		/*先断开连接*/
		ret = AT_send_cmd("AT+CIPCLOSE",(int)strlen("AT+CIPCLOSE"), NULL, 0, 2000);
		if(ret)
		{
			printf("\r\nAT+CIPCLOSE faliue,ret = %d\r\n",ret);
		}
		/*1. 配置 WiFi 模式*/
		ret = AT_send_cmd("AT+CWMODE=3",(int)strlen("AT+CWMODE=3"),NULL,NULL,2000);
		if(ret)
		{
			printf("\r\nAT+CWMODE=3 faliue,ret = %d\r\n",ret);
		}
		/*2.1断开原有的连接*/
		ret = platform_net_socket_close(ret);
		if(ret)
		{
			printf("\r\nWIFI disconnet faliue,ret = %d\r\n",ret);
		}
		/*2.2连接路路由器*/
		ret = AT_send_cmd("AT+CWJAP=\"" JAP_SSID "\",\"" JAP_passwoed "\"",(int)strlen("AT+CWJAP=\"" JAP_SSID "\",\"" JAP_passwoed "\""),NULL,NULL,20000);
		if(ret)
		{
			printf("\r\nWIFI connect faliue,ret = %d\r\n",ret);
			continue;
		}	
		/*3. ESP8266 设备作为 TCP client 连接到上述服务器 例子:(AT+CIPSTART="TCP","192.168.3.116",8080) */
		if(proto == PLATFORM_NET_PROTO_TCP)
		{
			sprintf(cmd,"AT+CIPSTART=\"TCP\",\"%s\",%s",host,port);
		}
		else //proto == PLATFORM_NET_PROTO_UDP
		{
			sprintf(cmd,"AT+CIPSTART=\"UDP\",\"%s\",%s",host,port);
		}
		ret = AT_send_cmd(cmd,(int)strlen(cmd),NULL,NULL,10000);
		if(ret)
		{
			printf("\r\nTCP connect faliue,ret = %d\r\n",ret);
			continue;
		}
		if(!ret)
		{
			break;
		}
	}
	/*连接成功*/
	return 0;
}


int platform_net_socket_recv_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	int i=0;
	int err;
	while(i<len)
	{
		err = AT_Recv_data((char*)&buf[i],timeout);
		if(err)
		{
			return err;
		}
		i++;
	}
	return len;
}

int platform_net_socket_write_timeout(int fd, unsigned char *buf, int len, int timeout)
{
	volatile int ret;
	char cmd[20]={0};
	sprintf(cmd,"AT+CIPSEND=%d",len);
		/*1.发AT命令AT+CIPSEND=*/
		ret = AT_send_cmd(cmd,(int)strlen(cmd),NULL,NULL,timeout);
		if(ret)
		{
			printf("\r\nAT+CIPSEND faliue,ret = %d\r\n",ret);
			return ret;
		}
	/*2.发送数据*buf*/ /*3.收响应*/
	ret = AT_send_data((char*)buf,len,timeout);
	if(ret<0)
	{
		printf("\r\nSend data faliue,ret = %d\r\n",ret);
		return ret;
	}
	/*成功写*/
	return len;
}

int platform_net_socket_close(int fd)
{
  return 	AT_send_cmd("AT+CWQAP",(int)strlen("AT+CWQAP"),NULL,NULL,2000);
}

#if 0
int platform_net_socket_recv(int fd, void *buf, size_t len, int flags)
{

}

int platform_net_socket_write(int fd, void *buf, size_t len)
{
	
}

int platform_net_socket_set_block(int fd)
{

}

int platform_net_socket_set_nonblock(int fd)
{

}

int platform_net_socket_setsockopt(int fd, int level, int optname, const void *optval,uint32_t optlen)
{

}
#endif
