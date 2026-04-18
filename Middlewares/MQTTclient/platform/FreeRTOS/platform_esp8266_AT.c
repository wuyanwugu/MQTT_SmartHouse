/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-01-10 23:45:59
 * @LastEditTime: 2020-04-25 17:50:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include "platform_mutex.h"
#include "cmsis_os2.h"
#include "platform_esp8266_AT.h"
#include "usart.h"
#include "HAL_AT.h"
#include <string.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"

extern circle_buf	g_circle_buff;//串口3环形buff结构体
circle_buf g_AT_process_data_buff;//AT接收数据Processr_recv_data()函数环形buff结构体
static uint8_t g_process_data_buff[AT_RESP_LEN];//Processr_recv_data()函数环形buff

platform_mutex_t g_at_send_mutex; 			//AT_send_cmd()函数二值信号量
platform_mutex_t g_at_rece_mutex; 			//AT_Recv_data()函数二值信号量
SemaphoreHandle_t g_at_send_mutex_x; 			//HAL_AT_SENDcmd()函数二值信号量

static int g_AT_status;       			//AT命令回复值变量
static char g_AT_resp[AT_RESP_LEN];			//AT回复数据存储数组
static char *g_AT_current_cmd;				 //当前要发送的AT命令
enum AT_Typte g_at_typte;							 //AT命令的类型定义

/*AT命令初始化*/
void AT_Init(void)
{
	g_at_send_mutex_x = xSemaphoreCreateMutex();														 //初始化HAL_AT_SENDcmd()互斥量
	platform_mutex_init(&g_at_send_mutex);																	 //初始化AT_send_data()二值信号量，用于与AT_Parse()任务的同步
	platform_mutex_lock(&g_at_send_mutex);																	 //mutex = 0;
	platform_mutex_init(&g_at_rece_mutex);																	 //初始化AT_Recv_data()二值信号量
  platform_mutex_lock(&g_at_rece_mutex);																	 //mutex = 0;
	circle_buf_init(&g_AT_process_data_buff,AT_RESP_LEN,g_process_data_buff);//初始化Processr_recv_data()函数环形buff
}

/*获取AT命令是OK还是ERR*/
int GetATstatus(void)
{
	return g_AT_status;
}

/*赋予AT命令是OK还是ERR*/
void GiveATstatus(int status)
{
	g_AT_status = status;
}

/*从Processr_recv_data()函数环形buff读出数据*/
int AT_Recv_data(char *c,unsigned int timeout)
{
	while(1)
	{
		if(0 == circle_buf_read(&g_AT_process_data_buff,(uint8_t*)c))
		{
			return AT_OK;
		}
		else
		{
			if(0 == platform_mutex_lock_timeout(&g_at_rece_mutex,timeout))
			{
				return AT_TIMEOUT;
			}
		}
	}
}

/*AT数据发送函数*/
int AT_send_data(char* data ,int data_len,int timeout)
{
	int ret;
	int err;
	/*发送命令*/
	HAL_AT_SENDcmd((const unsigned char*)data,data_len,timeout);
	g_AT_current_cmd = data;
	/*返回发送结果
	 * OK 0 :
	 * 		 
	 * ERR -1
	 * 超时 -2
	 */
	ret = platform_mutex_lock_timeout(&g_at_send_mutex,timeout);
	if(ret) //有结果发回来
	{
		/*判断返回值*/
		/*存储resp*/
		err = GetATstatus();
		return err;
	}
	else    //超时了还没结果发回来
	{
		return AT_TIMEOUT;
	}
	
}

/* AT命令发送函数
 * 
 * 
 */
int AT_send_cmd(char* cmd ,int cmd_len,char* resp,int resp_len,int timeout)
{
	int ret;
	int err;
	/*发送命令*/
	
	HAL_AT_SENDcmd((const unsigned char*)cmd,cmd_len,timeout);
	HAL_AT_SENDcmd((const unsigned char*)"\r\n",2,1000);
	g_AT_current_cmd = cmd;
	/*返回发送结果
	 * OK 0 : 有resp 
	 * 		 无resp
	 * ERR -1
	 * 超时 -2
	 */
	ret = platform_mutex_lock_timeout(&g_at_send_mutex,timeout);
	if(ret) //有结果发回来
	{
		/*判断返回值*/
		/*存储resp*/
		err = GetATstatus();
		
		if(!err && resp)
		{
			memcpy(resp,g_AT_resp,resp_len>AT_RESP_LEN ?AT_RESP_LEN :resp_len);
			
		}
		return err;
	}
	else    //超时了还没结果发回来
	{
		return AT_TIMEOUT;
	}
	
}

/*收到特殊字符回复*/
int Getspecialstring(char*buf)
{
	/*1. ESP8266 设备接收到服务器发来的数据*/
	if(strstr(buf,"+IPD,"))
	{
		g_at_typte = Recevie_Data;
		return g_at_typte;
	}
	/*2.ESP8266 设备向服务器发送数据*/
	else if(g_AT_current_cmd && strstr(g_AT_current_cmd,"AT+CIPSEND=")&&buf[0] == '>')
	{
		g_at_typte = Send_Data;
		return g_at_typte;
	}
	/*3.ESP8266 设备向服务器发送数据,但输入的字节数超过了了设置长度*/
	else if(strstr(buf,"busy")&&strstr(buf,"SEND OK"))
	{
		g_at_typte = Send_Data_Busy;
		return g_at_typte;
	}
	else
	{
		return 0;
	}
}

/*解析从服务器接收来的数据*/
/* +IPD,78:xxxxxxxxxx */
void Processr_recv_data(char*buf)
{
	int i = 0;
	int len = 0;
	while(1)
	{
		HAL_AT_Recv(&buf[i],portMAX_DELAY);
		/*1.解析数据长度*/
		if(i && buf[i] == ':')
		{
			break;
		}
		else
		{
			len = len * 10 + (buf[i] - '0');
		}
		i++;
	}
	i = 0;
	if(len>=AT_RESP_LEN)
	{
		printf("\r\nReceive data too large!\r\n");
		memset(buf, 0, sizeof(*buf)); // 使用memset清空数组
		GiveATstatus(AT_ERR);
		return;
	}
	while(i<len)
	{
		HAL_AT_Recv(&buf[i],portMAX_DELAY);
		circle_buf_write(&g_AT_process_data_buff,buf[i]);
		platform_mutex_unlock(&g_at_rece_mutex);
		i++;
	}
	
}
/*AT解析任务*/
void AT_Parse(void*parmas)
{
	static char rx_buf[AT_RESP_LEN]={0};
	uint8_t i = 0;

	while(1)
	{
		 /*HAL层读数据阻塞*/
			HAL_AT_Recv(&rx_buf[i],portMAX_DELAY);
		  rx_buf[i+1] = '\0';

			/*有接收到数据，解析数据*/
			if(i && (rx_buf[i] == '\n') && (rx_buf[i-1] == '\r'))
			{
				if(strstr(rx_buf,"OK\r\n"))
				{
					/*发送数据到服务器,但输入的字节数超过了了设置长度*/
					if(Getspecialstring(rx_buf)==Send_Data_Busy)
					{
						i = 0;
						printf("send busy\r\n");
						GiveATstatus(AT_OK);
						memset(rx_buf, 0, sizeof(rx_buf)); // 使用memset清空数组
						platform_mutex_unlock(&g_at_send_mutex);
					}
//					printf("OK\r\n");
					else
					{
						i = 0;
						GiveATstatus(AT_OK);
						memset(rx_buf, 0, sizeof(rx_buf)); // 使用memset清空数组
						platform_mutex_unlock(&g_at_send_mutex);
						
					}
				}
				else if(strstr(rx_buf,"ERROR\r\n"))
				{
					i = 0;
//					printf("EEROR\r\n");
					GiveATstatus(AT_ERR);
					memset(rx_buf, 0, sizeof(rx_buf)); // 使用memset清空数组
					platform_mutex_unlock(&g_at_send_mutex);
				}

				i = 0;
		  }
			/*接收到服务器发来的数据*/
			else if(Getspecialstring(rx_buf) == Recevie_Data)
			{
				Processr_recv_data(rx_buf);        // 处理接收数据函数
				i = 0;
				memset(rx_buf, 0, sizeof(rx_buf)); // 使用memset清空数组
			}
//			/*发送数据到服务器*/
//			else if(Getspecialstring(rx_buf) == Send_Data)
//			{
//				i = 0;
//				memset(rx_buf, 0, sizeof(rx_buf));  使用memset清空数组
//				GiveATstatus(AT_OK);
//				platform_mutex_unlock(&g_at_send_mutex);
//			}
			else
			{
				i++;
			}
			if(i>=AT_RESP_LEN)
			{
				
				i = 0;
			}
	}
}


