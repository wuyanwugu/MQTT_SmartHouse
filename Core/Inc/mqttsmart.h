#ifndef __MQTTSMART_H__
#define __MQTTSMART_H__
#include "mqttclient.h"
// 定义智能家居指令类型枚举
typedef enum {
    SMART_TYPE_UNKNOWN = 0,
    SMART_TYPE_LED,
    SMART_TYPE_BUZZER,
    SMART_TYPE_SG90,
    SMART_TYPE_DHT11,
    SMART_TYPE_MPU6050,
    SMART_TYPE_COLOR_LED,
    SMART_TYPE_OLED,
    // 可以根据需要添加更多的智能设备类型
} SMART_Type_e;
// 智能家居指令识别表
typedef struct {
    const char* command; // 指令字符串
    SMART_Type_e type;   // 对应的智能设备类型
} SMART_Command_t;



// 定义智能家居指令解析结果
typedef struct {
    SMART_Type_e type;           // 指令类型
    char params[32];       // 参数（最多32字符）
} SMART_Result_t;

void mqttsmart_parse(mqtt_client_t* c,char*msg);

#endif
