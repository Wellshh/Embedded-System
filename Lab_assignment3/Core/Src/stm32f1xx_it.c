/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_v4.h"
#include <stdio.h>
#include <string.h>
#include "KEY.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ROW 60
#define COLUMN 45
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t rxBuffer[20];
volatile uint8_t showFlag = 0;
unsigned short pic[2710];
char msg[100];
char new_msg[100];
uint8_t start_signal[1] = {0x00}; //成功接收
uint8_t cont_signal[1] = {0x11}; //继续发�??
uint8_t stop_signal[1] = {0x22}; //暂停发�??
uint8_t slow_signal[1] = {0x33};
uint8_t fast_signal[1] = {0x44};
uint8_t speed_state = 1;
static unsigned int cnt = 0;
uint8_t state = 0; //状�?�机
uint8_t size_flag = 1; //改变图片显示大小
uint8_t stop_flag = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(KEY_WK_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(KEY0_Pin);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
  HAL_UART_Receive_IT(&huart1,(uint8_t *)rxBuffer,1);
  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static unsigned short data;
	static unsigned int uLength = 0;
	if(huart->Instance==USART1){
		static uint32_t start_time = 0;
		switch(state){
		case 0 :
			data = rxBuffer[0];
			if(data == 0x34){
				state = 1;
			}
			break;
		case 1:
			data |= (rxBuffer[0] << 8);
			if (data == 0x1234){
				state = 2;
				uLength = 0;
			}
			else {
				state = 0;
				break;
			}
//			HAL_UART_Transmit(&huart1,get_signal,1,0xffff);
			start_time = HAL_GetTick();
			break;
		case 2:
			data = rxBuffer[0];
			state = 3;
			break;
		case 3:
			data |= (rxBuffer[0] << 8);
			pic[uLength] = data;
			uLength ++;
//			HAL_UART_Transmit(&huart1,get_signal,1,0xffff);
			if (uLength == 2700){
//				HAL_UART_Transmit(&huart1,get_signal,1,0xffff);
				lcd_showpic(0,0,60,45,pic,size_flag);
				state = 0;
				break;
			}
			else if(HAL_GetTick() - start_time < 500){
				state = 2;
			}
			else {state = 0;}
			break;
		}
//		HAL_UART_Transmit(&huart1,get_signal,1,0xffff);
//		char show_state[2] = {state + '0','\0'};
//		lcd_show_string(lcddev.width/10, lcddev.height * 3 /5, 200,16,16,show_state,BLACK);


//		if(data == 0x1234){
//			uLength = 0;
//			HAL_UART_Transmit(&huart1,cont_signal,1,0xffff);
//		}
//		if(uLength == 2700){
//			showFlag = 1;
//			uLength = 0;
//			lcd_clear(WHITE);g_back_color = WHITE;
//			lcd_showpic(0,0,COLUMN,ROW,pic);
//			HAL_UART_Transmit(&huart1,cont_signal,1,0xffff);
//		}
		if (stop_flag == 0 && speed_state == 0){ //慢�?�播�?
				HAL_UART_Transmit(&huart1,slow_signal,1,0xffff);
		}
		else if(stop_flag == 0 && speed_state == 1){//正常速度
			HAL_UART_Transmit(&huart1,cont_signal,1,0xffff);
		}
		else if(stop_flag == 0 && speed_state == 2){ //快进
			HAL_UART_Transmit(&huart1,fast_signal,1,0xffff);
		}

	}
//	__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC); // HAL 库提供的清除方法
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim3){
		if (stop_flag == 0){
		cnt++;
		}
		sprintf(msg,"Time Elapsed is %d\r", cnt);
		lcd_show_string(lcddev.width/10,lcddev.height * 9/10,200,16,16,msg, BLACK);
//		HAL_UART_Transmit(&huart1,cont_signal,1,0xffff); //结束中断，继续UART传输
	}
	__HAL_TIM_CLEAR_IT(htim,TIM_IT_UPDATE);

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == KEY_WK_Pin){
		HAL_Delay(10);
		if (HAL_GPIO_ReadPin(KEY_WK_GPIO_Port, KEY_WK_Pin) == 1){
			lcd_clear(WHITE); g_back_color = WHITE;
			HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
			switch(size_flag){
			case 1:
				size_flag = 2;
				break;
			case 2:
				size_flag = 3;
				break;
			case 3:
				size_flag = 1;
				break;
			}
		}
	}
	else if(GPIO_Pin == KEY1_Pin){
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) == 0 && size_flag != 3){
			lcd_clear(WHITE); g_back_color = WHITE;
			stop_flag = !stop_flag;
			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
			HAL_UART_Transmit(&huart1,start_signal,1,0xffff);
		}
		else if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) == 0 && size_flag == 3){
			lcd_display_off();
		}
	}
	else if(GPIO_Pin == KEY0_Pin){
		HAL_Delay(10);
		if (HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin) == 0 && size_flag != 3){
			lcd_clear(WHITE); g_back_color = WHITE;
			unsigned short pic_cp[2710];
			memcpy(pic_cp,pic,sizeof(pic));
			lcd_showpic(0,lcddev.height*3/5 - 50,60,45,pic_cp,size_flag);
			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
		}
		else if(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin) == 0 && size_flag == 3){
			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
			switch(speed_state){
			case 0:
				lcd_clear(WHITE); g_back_color = WHITE;
				speed_state = 1;
				lcd_show_string(lcddev.width/10,lcddev.height * 7/10,200,24,24,"Normal Speed!", BLACK);
				break;
			case 1:
				lcd_clear(WHITE); g_back_color = WHITE;
				speed_state = 2;
				lcd_show_string(lcddev.width/10,lcddev.height * 7/10,200,24,24,"2 x Faster!", BLACK);
				break;
			case 2:
				lcd_clear(WHITE); g_back_color = WHITE;
				speed_state = 0;
				lcd_show_string(lcddev.width/10,lcddev.height * 7/10,200,24,24,"0.5 x Slower!", BLACK);
				break;
			}
		}
	}
	HAL_Delay(100);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
    {
      if (HAL_UART_GetError(huart) & HAL_UART_ERROR_ORE)
        __HAL_UART_FLUSH_DRREGISTER(huart);
    }
/* USER CODE END 1 */
