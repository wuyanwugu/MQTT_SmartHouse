#include "stm32f1xx_hal.h"
#include "tim.h"

/**********************************************************************
 * 函数名称： sg90_Init
 * 功能描述： SG90的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void sg90_Init(void)
{
    /* TIM2_CH1在MX_TIM2_Init中被配置为PWM输出 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}
/**********************************************************************
 * 函数名称： sg90_SetAngle
 * 功能描述： SG90设置角度函数
 * 输入参数： angle - 角度值,范围为 0-180
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void sg90_SetAngle(unsigned int angle)
{
    if (angle > 180) angle = 180;
    
    uint16_t pwm_duty = 100 + (angle * 400) / 180;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_duty);
}
//    // 测试最小值
//    sg90_SetAngle(0);
//    vTaskDelay(1000);
//    
//    // 测试中间值
//    sg90_SetAngle(90);
//    vTaskDelay(1000);
//    
//    // 测试最大值
//    sg90_SetAngle(180);
//    vTaskDelay(1000);	
