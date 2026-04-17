#include "HAL_AT.h"
#include "usart.h"
#include "platform_mutex.h"
extern circle_buf	g_circle_buff;
extern SemaphoreHandle_t g_at_send_mutex_x; 			//HAL_AT_SENDcmd()函数二值信号量
/*
 * @brief HAL层 发送AT命令
 * @param cmd: 要发送的AT命令
 * @param len: 命令长度
 * @retval None
 */
void HAL_AT_SENDcmd(const unsigned char *cmd,int len,int timeout)
{
		xSemaphoreTake(g_at_send_mutex_x,portMAX_DELAY);
    //串口3发送AT命令
    uart3_write(cmd,len,timeout);
		xSemaphoreGive(g_at_send_mutex_x);
}
/* 
 * @brief HAL层 收AT命令回复
 * @param c: 接收数据字符
 * @param timeout: 阻塞时间
 * @retval None
 */
void HAL_AT_Recv(char *c,unsigned int timeout)
{
	while(1)
	{
		if(0 == circle_buf_read(&g_circle_buff,(uint8_t*)c))
		{
			return;
		}
		else
		{
			ulTaskNotifyTake(pdTRUE,timeout);
		}
	}
}
