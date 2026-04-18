#include "driver_led.h"
#include "driver_lcd.h"
#include "driver_mpu6050.h"
#include "driver_dht11.h"
#include "driver_passive_buzzer.h"
#include "driver_color_led.h"
#include "driver_sg90.h"
#include <string.h>
/**********************************************************************
 * 函数名称： mqttsmart_parse
 * 功能描述： 解析订阅MQTTSmart主题的消息，对外设进行操作
 * 输入参数： char(char*)msg->message->payload
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void mqttsmart_parse(char*msg)
{
    /* 标识符{} */
    if(strstr(msg,"{")&&strstr(msg,"}"))
    {
        if (strstr(msg, "led_on"))
        {
            Led_Control(LED_GREEN, 1);
        }
        else if (strstr(msg, "led_off"))
        {
            Led_Control(LED_GREEN, 0);
        }
        else if (strstr(msg, "buzzer_on"))
        {
            PassiveBuzzer_Control(1);
        }
        else if (strstr(msg, "buzzer_off"))
        {
            PassiveBuzzer_Control(0);
        }
    }
}
