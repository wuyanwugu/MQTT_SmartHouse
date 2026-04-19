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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const SMART_Command_t g_smart_command_table[] = {
    {"led", SMART_TYPE_LED},//led指令格式 eg.{led_on} {led_off}
    {"buzzer", SMART_TYPE_BUZZER},//buzzer指令格式 eg.{buzzer_on} {buzzer_off}
    {"sg90", SMART_TYPE_SG90},//sg90指令格式 eg.{sg90_angle_90}
    {"dht11", SMART_TYPE_DHT11},//dht11指令格式 eg.{dht11_read}
    {"mpu6050", SMART_TYPE_MPU6050},//mpu6050指令格式 eg.{mpu6050_read}
    {"color_led", SMART_TYPE_COLOR_LED},//color_led指令格式 eg.{color_led_r} {color_led_g} {color_led_b}
    {"oled", SMART_TYPE_OLED},
    // 可以根据需要添加更多的指令和对应的设备类型(目前7个)
};
/**********************************************************************
 * 函数名称： mqttsmart_msg_parse
 * 功能描述： 解析订阅MQTTSmart主题的消息，对外设进行操作
 * 输入参数： SMART_Result_t result
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void mqttsmart_msg_parse(SMART_Result_t result)
{
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
						
            break;
        case SMART_TYPE_MPU6050:
            // 解析MPU6050指令参数并读取加速度和陀螺仪数据
						/*互斥量读数据*/
            break;
        case SMART_TYPE_COLOR_LED:
            // 解析全彩LED指令参数并控制全彩LED
            break;
        case SMART_TYPE_OLED:
            // 解析OLED指令参数并控制OLED显示
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
void mqttsmart_parse(char*msg)
{
    SMART_Result_t result;
    memset(&result, 0, sizeof(result));
    unsigned char i = 0;

    /* 标识符{} */
    if(strstr(msg,"{")&&strstr(msg,"}"))
    {
        for(i = 0;i<sizeof(g_smart_command_table)/sizeof(g_smart_command_table[0]);i++)
        {
            if (strstr(msg, g_smart_command_table[i].command))
            {
                result.type = g_smart_command_table[i].type;
                strncpy(result.params, msg,31);
                result.params[31] = '\0'; // 确保字符串以null结尾
                break;
            }
        }
        if(i >= sizeof(g_smart_command_table)/sizeof(g_smart_command_table[0]))
        {
 
            result.type = SMART_TYPE_UNKNOWN;
            printf("Unknown command received: %s\r\n", msg);
            return;
        }
        else
        {
            mqttsmart_msg_parse(result);
						
        }
    }
    else
    {
        result.type = SMART_TYPE_UNKNOWN;
        printf("Invalid command format received: %s\r\n", msg);
        return;
    }
}
