/* USER CODE BEGIN Header */
#include "driver_led.h"
#include "driver_lcd.h"
#include "driver_mpu6050.h"
#include "driver_timer.h"
#include "driver_ds18b20.h"
#include "driver_dht11.h"
#include "driver_active_buzzer.h"
#include "driver_passive_buzzer.h"
#include "driver_color_led.h"
#include "driver_ir_receiver.h"
#include "driver_ir_sender.h"
#include "driver_light_sensor.h"
#include "driver_ir_obstacle.h"
#include "driver_ultrasonic_sr04.h"
#include "driver_spiflash_w25q64.h"
#include "driver_rotary_encoder.h"
#include "driver_motor.h"
#include "driver_key.h"
#include "driver_uart.h"
#include "driver_sg90.h"
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "semphr.h"
#include "platform_esp8266_AT.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern SemaphoreHandle_t g_mpu6050_mutex; //MPU6050互斥量(在mqttsmart.c中定义)
extern SemaphoreHandle_t g_DHT11_mutex; //DHT11互斥量(在mqttsmart.c中定义)
extern SemaphoreHandle_t g_OLED_mutex; //OLED互斥量(在mqttsmart.c中定义)

TaskHandle_t AT_pars_handle;//AT指令分析任务句柄
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/*硬件初始化函数*/
void BSP_Init(void)
{
   //LED初始化（在main.c中MX_GPIO_Init()初始化）
	LCD_Init();  //OLED初始化
	LCD_Clear();
	sg90_Init(); //舵机初始化
	MPU6050_Init();//MPU6050初始化
  DHT11_Init(); //DHT11初始化
  PassiveBuzzer_Init();//无源蜂鸣器初始化
  PassiveBuzzer_Control(0);
	ColorLED_Set_NoGreen(0);//全彩LED初始化
}
void mqttsmart_mutex_init(void)
{
  g_mpu6050_mutex = xSemaphoreCreateMutex();
  g_DHT11_mutex = xSemaphoreCreateMutex();
  g_OLED_mutex = xSemaphoreCreateMutex();
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	AT_Init();//AT命令相关程序初始化
			/*硬件初始化*/
  BSP_Init();
  mqttsmart_mutex_init();//智能家居指令相关互斥量初始化
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */

  //defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
	
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	extern void MQTT_Task(void*parm);
	xTaskCreate(AT_Parse,"AT_parse",68,NULL,osPriorityNormal,&AT_pars_handle);
	xTaskCreate(MQTT_Task,"MQTT_Task",150,NULL,osPriorityNormal,NULL);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  //sg90_Init(); //舵机初始化
	for(;;)
	{
		    // 测试最小值
    sg90_SetAngle(0);
    vTaskDelay(1000);
    
    // 测试中间值
    sg90_SetAngle(90);
    vTaskDelay(1000);
    
    // 测试最大值
    sg90_SetAngle(180);
    vTaskDelay(1000);	
//		vTaskDelay(100);
    //Led_Test();
    //LCD_Test();
	//MPU6050_Test(); 
	//DS18B20_Test();
	//DHT11_Test();
	//ActiveBuzzer_Test();
	//PassiveBuzzer_Test();
	//ColorLED_Test();
	//IRReceiver_Test();
	//IRSender_Test();
	//LightSensor_Test();
	//IRObstacle_Test();
	//SR04_Test();
	//W25Q64_Test();
	//RotaryEncoder_Test();
	//Motor_Test();
	//Key_Test();
	//UART_Test();
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*----------------打印所有任务空闲栈大小----------------------*/
//static char g_task_infor[200];
//void vApplicationIdleHook(void*parm)
//{
//	vTaskList(g_task_infor);
//		for(int i = 0 ;i<16;i++)
//	{
//		printf("-");
//	}
//	printf("\n\r\n\r");
//	printf("%s\n\r",g_task_infor);
//}
/* USER CODE END Application */

