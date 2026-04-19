#include "driver_led.h"
#include "driver_lcd.h"
#include "driver_mpu6050.h"
#include "driver_dht11.h"
#include "driver_passive_buzzer.h"
#include "driver_color_led.h"
#include "driver_sg90.h"
#include "mqttsmart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "driver_timer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

SemaphoreHandle_t g_mpu6050_mutex; //MPU6050互斥量
SemaphoreHandle_t g_DHT11_mutex; //DHT11互斥量
SemaphoreHandle_t g_OLED_mutex; //OLED互斥量

static char buf[100] = { 0 };

static const SMART_Command_t g_smart_command_table[] = {
    {"led_", SMART_TYPE_LED},//led指令格式 eg.{led_on} {led_off}
    {"buzzer", SMART_TYPE_BUZZER},//buzzer指令格式 eg.{buzzer_on} {buzzer_off}
    {"sg90", SMART_TYPE_SG90},//sg90指令格式 eg.{sg90_angle_90}
    {"dht11", SMART_TYPE_DHT11},//dht11指令格式 eg.{dht11_read}
    {"mpu6050", SMART_TYPE_MPU6050},//mpu6050指令格式 eg.{mpu6050_read}
    {"COLOR_LED", SMART_TYPE_COLOR_LED},//COLOR_LED指令格式 eg.{COLOR_LED_r} {COLOR_LED_g} {COLOR_LED_b}
    {"OLED", SMART_TYPE_OLED},//OLED指令格式 {OLED_print_x_y_text} eg.{OLED_print_1_2_Hello} {OLED_clear}
    // 可以根据需要添加更多的指令和对应的设备类型(目前7个)
};
/**********************************************************************
 * 函数名称： mqttsmart_msg_parse
 * 功能描述： 解析订阅MQTTSmart主题的消息，对外设进行操作
 * 输入参数： SMART_Result_t result
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void mqttsmart_msg_parse(mqtt_client_t* c,SMART_Result_t result)
{
    mqtt_message_t publish_info;
	publish_info.qos = 0;
    publish_info.payload = buf;
    if(*buf != NULL)
    {
        memset(buf, 0, sizeof(buf));
    }
	static int hum = 0, temp = 0;//dht11数据存放
    static int16_t AccX = 0, AccY = 0, AccZ = 0, GyroX = 0, GyroY = 0, GyroZ = 0;//mp6050数据存放
    switch(result.type)
    {
        case SMART_TYPE_LED:
            // 解析LED指令参数并控制LED
            if(strstr(result.params,"on"))
            {
                Led_Control(1,1);
            }
            else if(strstr(result.params,"off"))
            {
                Led_Control(1,0);
            }
        break;

        case SMART_TYPE_BUZZER:
            // 解析蜂鸣器指令参数并控制蜂鸣器
            if(strstr(result.params,"on"))
            {
                PassiveBuzzer_Control(1);
            }
            else if(strstr(result.params,"off"))
            {
                PassiveBuzzer_Control(0);
            }
        break;

        case SMART_TYPE_SG90:
            // 解析SG90指令参数并控制舵机
            if(strstr(result.params,"angle_"))
            {
                char* angle_str = strstr(result.params,"angle_") + strlen("angle_");
                int angle = atoi(angle_str);
                sg90_SetAngle(angle);
            }
        break;

        case SMART_TYPE_DHT11:
            // 解析DHT11指令参数并读取温湿度数据
			/*互斥量读数据*/
            if(xSemaphoreTake(g_DHT11_mutex, 0)==pdFALSE)
            {
                printf("DHT11 is busy, please try again later.\r\n");
                sprintf(buf, "DHT11 is busy, please try again later.\r\n");
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
                return;
            }
            if(DHT11_Read(&hum, &temp)==0)
            {
                printf("DHT11 read success: humidity = %d%%, temperature = %d*C\r\n", hum, temp);
                sprintf(buf, "DHT11 read success: humidity = %d%%, temperature = %d*C\r\n", hum, temp);
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
            }
            else
            {
                printf("DHT11 read failed\r\n");
                sprintf(buf, "DHT11 read failed\r\n");
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
            }
            xSemaphoreGive(g_DHT11_mutex);
        break;

        case SMART_TYPE_MPU6050:
            // 解析MPU6050指令参数并读取加速度和陀螺仪数据
			/*互斥量读数据*/
            if(xSemaphoreTake(g_mpu6050_mutex, 0)==pdFALSE)
            {
                printf("MPU6050 is busy, please try again later.\r\n");
                sprintf(buf, "MPU6050 is busy, please try again later.\r\n");
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
                return;
            }
            if(MPU6050_ReadData(&AccX, &AccY, &AccZ, &GyroX, &GyroY, &GyroZ)==0)
            {
                printf("MPU6050 read success: AccX = %d, AccY = %d, AccZ = %d, GyroX = %d, GyroY = %d, GyroZ = %d\r\n", AccX, AccY, AccZ, GyroX, GyroY, GyroZ);
                sprintf(buf, "MPU6050 read success: AccX = %d, AccY = %d, AccZ = %d, GyroX = %d, GyroY = %d, GyroZ = %d\r\n", AccX, AccY, AccZ, GyroX, GyroY, GyroZ);
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
            }
            else            
            {
                printf("MPU6050 read failed\r\n");
                sprintf(buf, "MPU6050 read failed\r\n");
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
            }
            xSemaphoreGive(g_mpu6050_mutex);
        break;

        case SMART_TYPE_COLOR_LED:
            // 解析全彩LED指令参数并控制全彩LED
            if(strstr(result.params,"COLOR_LED_"))
            {
                char* color_str = strstr(result.params,"COLOR_LED_") + strlen("COLOR_LED_");
                uint32_t color = strtoul(color_str, NULL, 16);
                ColorLED_Set_NoGreen(color);
            }
        break;

        case SMART_TYPE_OLED:
            // 解析OLED指令参数并控制OLED显示
            if(xSemaphoreTake(g_OLED_mutex, 0)==pdFALSE)
            {
                printf("OLED is busy, please try again later.\r\n");
                sprintf(buf, "OLED is busy, please try again later.\r\n");
                publish_info.payloadlen = strlen(buf);
                mqtt_publish(c, "test_task", &publish_info);
                return;
            }
            LCD_Clear();
            if(strstr(result.params,"OLED_print_"))
            {
                //OLED_print_x_y_text
                char* text = strstr(result.params,"OLED_print_") + strlen("OLED_print_");
                char* x_str = strtok(text, "_");
                char* y_str = strtok(NULL, "_");
                int x = atoi(x_str);
                int y = atoi(y_str);
                text = strtok(NULL, "_");
                if(text != NULL)
                {
                    char* last_brace = strrchr(text, '}');
                    if(last_brace != NULL)
                    {
                        *last_brace = '\0';
                    }
                }
                LCD_PrintString(x, y, text);
            }
            else if(strstr(result.params,"OLED_clear"))
            {
                LCD_Clear();
            }
            xSemaphoreGive(g_OLED_mutex);
        break;
        default:
            
            break;
    }
}
/**********************************************************************
 * 函数名称： mqttsmart_parse
 * 功能描述： 解析订阅MQTTSmart主题的消息，对外设进行操作
 * 输入参数： char(char*)msg->message->payload
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void mqttsmart_parse(mqtt_client_t* c,char*msg)
{
    SMART_Result_t result;
    memset(&result, 0, sizeof(result));
    unsigned char i = 0;
    mqtt_message_t publish_info;
	publish_info.qos = 0;
    publish_info.payload = buf;
    if(*buf != NULL)
    {
        memset(buf, 0, sizeof(buf));
    }
    /* 标识符{} */
    if(strstr(msg,"{")&&strstr(msg,"}"))
    {
        for(i = 0;i<sizeof(g_smart_command_table)/sizeof(g_smart_command_table[0]);i++)
        {
            if (strstr(msg, g_smart_command_table[i].command))
            {
                result.type = g_smart_command_table[i].type;
                strncpy(result.params, msg,31); //最多复制31个字符，留一个位置给null终止符
                result.params[31] = '\0'; // 确保字符串以null结尾
                break;
            }
        }
        if(i > sizeof(g_smart_command_table)/sizeof(g_smart_command_table[0]))
        {
 
            result.type = SMART_TYPE_UNKNOWN;
            sprintf(buf, "Unknown command received: %s\r\n", msg);
            publish_info.payloadlen = strlen(buf);
            mqtt_publish(c, "test_task", &publish_info);
            printf("Unknown command received: %s\r\n", msg);
            return;
        }
        else
        {
            mqttsmart_msg_parse(c,result);
						
        }
    }
    else
    {
        result.type = SMART_TYPE_UNKNOWN;
        printf("Invalid command format received: %s\r\n", msg);
        sprintf(buf, "Invalid command format received: %s\r\n", msg);
        publish_info.payloadlen = strlen(buf);
        mqtt_publish(c, "test_task", &publish_info);
        return;
    }
}
