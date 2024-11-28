/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include "lcd_v4.h"
#include "KEY.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LENGTH 10
#define HAMMING_MIN_LENGTH 3
#define HAMMING_MAX_LENGTH 14

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
char* HAMMING_ENCODE(char* input);
char* HAMMING_DECODE(char* hamming_input);
uint8_t char_to_bit(char c);
char bit_to_char(uint8_t bit);
uint8_t calculate_redundant_bits(uint8_t input_len, uint8_t mode);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static char wr_data[11]; //纠错前的数据
static char parity_val[4]; //记录校验位的计算结果
static uint8_t error_position = 0; //Decode模式下错误的位置，默认为0

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  lcd_init();

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  Key_Config();
  HAL_TIM_Base_Start_IT(&htim2);//开定时器
  static uint8_t bit_len = 4; //默认original数据长度是4位
  static uint8_t hamming_len = 7;//默认hamming码长度是7
  static uint8_t mode = 1;
  static char or_data[11];
  static char or_hamming[15];
  char* encoded_data;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(button[2].buttonAction == BUTTON_LONG_PRESS){//长按KEY_WK_UP设置original数据长度
		  lcd_clear(WHITE);g_back_color = WHITE;
		  button[2].buttonAction = BUTTON_NULL;
		  char len_str[4];
		  sprintf(len_str,"%d",bit_len);
		  lcd_show_string(10,lcddev.height/10,200,24,24,"Input Length: ",BLACK);
		  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
		  lcd_show_string(10,lcddev.height/10 + 50,200,16,16,"Maximum Length is 10", BLACK);
		  while(button[2].buttonAction != BUTTON_LONG_PRESS){
			  //按KEY1减1，按KEY0加1
			  if(button[1].buttonAction == BUTTON_SINGLE){
				  button[1].buttonAction = BUTTON_NULL;
				  if(bit_len - 1 >= 1){
					  bit_len -= 1;
					  lcd_clear_area(WHITE,5*lcddev.width/6,lcddev.height/10,200,24);
					  sprintf(len_str,"%d",bit_len);
					  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
				  }
			  }
			  else if(button[0].buttonAction == BUTTON_SINGLE){
				  button[0].buttonAction = BUTTON_NULL;
				  if(bit_len + 1 <= MAX_LENGTH){
					  bit_len += 1;
					  lcd_clear_area(WHITE,5*lcddev.width/6,lcddev.height/10,200,24);
					  sprintf(len_str,"%d",bit_len);
					  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
				  }
				  else {
					  lcd_show_string(10, lcddev.height/10 + 100, 200, 24, 24, "Exceeding Maximum Length!", RED);
					  HAL_Delay(2000);
					  lcd_clear_area(WHITE,10,lcddev.height/10 + 100, 200,40);
				  }
			  }
		  }//再次长按KEY_WK_UP退出设置模式
		  button[2].buttonAction = BUTTON_NULL;
		  lcd_clear(WHITE);g_back_color = WHITE;
	  }
	  else if(button[2].buttonAction == BUTTON_DOUBLE){
		  lcd_clear(WHITE);g_back_color = WHITE;
		  button[2].buttonAction = BUTTON_NULL;
		  char len_str[4];
		  sprintf(len_str,"%d",hamming_len);
		  lcd_show_string(10,lcddev.height/10,200,24,24,"Hamming Length: ",BLACK);
		  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
		  lcd_show_string(10,lcddev.height/10 + 50,200,16,16,"Maximum Length is 14", BLACK);
		  while(button[2].buttonAction != BUTTON_DOUBLE){
			  //按KEY1减1，按KEY0加1
			  if(button[1].buttonAction == BUTTON_SINGLE){
				  button[1].buttonAction = BUTTON_NULL;
				  if(hamming_len - 1 >= HAMMING_MIN_LENGTH){
					  hamming_len -= 1;
					  lcd_clear_area(WHITE,5*lcddev.width/6,lcddev.height/10,200,24);
					  sprintf(len_str,"%d",hamming_len);
					  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
				  }
			  }
			  else if(button[0].buttonAction == BUTTON_SINGLE){
				  button[0].buttonAction = BUTTON_NULL;
					  if(hamming_len + 1 <= HAMMING_MAX_LENGTH){
						  hamming_len += 1;
						  lcd_clear_area(WHITE,5*lcddev.width/6,lcddev.height/10,200,24);
						  sprintf(len_str,"%d",hamming_len);
						  lcd_show_string(5 * lcddev.width/ 6, lcddev.height/10,200,24,24,len_str,BLACK);
					  }
					  else{
						  lcd_show_string(10, lcddev.height/10 + 100, 200, 24, 24, "Exceeding Maximum Length!", RED);
						  HAL_Delay(2000);
						  lcd_clear_area(WHITE,10,lcddev.height/10 + 100, 200,40);
					  }
				  }
			  }
		  button[2].buttonAction = BUTTON_NULL;
		  lcd_clear(WHITE);g_back_color = WHITE;
	  }//双击KEY_WK_UP设置HAMMING码长度
	  else{
		  //切换Encoding和Decoding模
		  if(button[2].buttonAction == BUTTON_SINGLE){
			  button[2].buttonAction = BUTTON_NULL;
			  if(mode == ENCODING){
				  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
				  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				  HAL_Delay(500);
				  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
				  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				  mode = DECODING;
			  }
			  else{
				  for(uint8_t i = 0; i < 3; i ++){
					  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
					  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
					  HAL_Delay(500);
				  }
				  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
				  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				  mode = ENCODING;
			  }
		  }
		  if(mode == ENCODING){
			  lcd_clear(WHITE);g_back_color = WHITE;
			  lcd_divide();
			  lcd_picture_mode(ENCODING);
			  uint8_t flag = 0; //flag检查是否已经输入完成
			  while(button[2].buttonAction != BUTTON_SINGLE && button[2].buttonAction != BUTTON_LONG_PRESS && button[2].buttonAction != BUTTON_DOUBLE){
				  uint8_t input_size = 0;
				  while(input_size < bit_len && flag == 0 && button[2].buttonAction != BUTTON_SINGLE && button[2].buttonAction != BUTTON_LONG_PRESS && button[2].buttonAction != BUTTON_DOUBLE){
					  if(button[0].buttonAction == BUTTON_SINGLE){//KEY0输入0，KEY1输入1
						  button[0].buttonAction = BUTTON_NULL;
						  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
						  HAL_Delay(500);
						  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
						  or_data[input_size] = '0';
						  input_size++;
					  }
					  else if(button[1].buttonAction == BUTTON_SINGLE){
						  button[1].buttonAction = BUTTON_NULL;
						  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
						  HAL_Delay(500);
						  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
						  or_data[input_size] = '1';
						  input_size++;
					  }
					  or_data[input_size] = '\0';
				  }
				  flag = 1;
				  lcd_show_string(10,lcddev.height/10 + 60, 200, 24,24,or_data,BLACK);
				  //Hamming Code编码
				  encoded_data = HAMMING_ENCODE(or_data);
				  lcd_show_string(10,2*lcddev.height/5 + 30, 200, 24,24,encoded_data,BLACK);
			  }

		  }
		  else {
			  lcd_clear(WHITE);g_back_color = WHITE;
			  lcd_divide();
			  lcd_picture_mode(DECODING);
			  uint8_t flag = 0;
			  //显示冗余位数量
			  char parity_bit[2];
			  parity_bit[0] = calculate_redundant_bits(hamming_len,DECODING) + '0';
			  parity_bit[1] = '\0';
			  char result[strlen("with ") + strlen(parity_bit) + strlen(" bits") + 1];  // 动态调整 result 大小
			  strcpy(result, "with ");
			  strcat(result, parity_bit);
			  strcat(result, " bits");
			  lcd_show_string(lcddev.width/2,2*lcddev.height/5+10,100,16,16,result,BLACK);
//			  lcd_show_string(10,lcddev.height/10 + 60, 200, 24,24,or_data,BLACK);
//			  if(encoded_data != NULL){lcd_show_string(10,2*lcddev.height/5 + 30, 200, 24,24,encoded_data,BLACK);}
			  while(button[2].buttonAction != BUTTON_SINGLE && button[2].buttonAction != BUTTON_LONG_PRESS && button[2].buttonAction != BUTTON_DOUBLE){
				  uint8_t input_size = 0;
				  while(input_size < hamming_len && flag == 0 && button[2].buttonAction != BUTTON_SINGLE && button[2].buttonAction != BUTTON_LONG_PRESS && button[2].buttonAction != BUTTON_DOUBLE ){
					  if(button[0].buttonAction == BUTTON_SINGLE){
						  button[0].buttonAction = BUTTON_NULL;
						  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
						  HAL_Delay(500);
						  HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
						  or_hamming[input_size] = '0';
						  input_size++;
					  }
					  else if(button[1].buttonAction == BUTTON_SINGLE){
						  button[1].buttonAction = BUTTON_NULL;
						  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
						  HAL_Delay(500);
						  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
						  or_hamming[input_size] = '1';
						  input_size++;
					  }
					  or_hamming[input_size] = '\0';
				  }
				  if(flag == 0){//只用显示一次
					  lcd_show_string(10,2*lcddev.height/5 + 30, 200, 24,24,or_hamming,BLACK);
					  char *decode = HAMMING_DECODE(or_hamming);
					  lcd_show_string(10,lcddev.height/10 + 60, 200, 24,24,decode,BLACK);
					  lcd_show_string(10,3*lcddev.height/5 + 10,100,16,16,"Parity Bits:",BLACK);
					  lcd_show_string(lcddev.width/2,3*lcddev.height/5+10,100,16,16,parity_val,BLACK);
					  if(error_position != 0){
						  lcd_show_string(10,3*lcddev.height/5+30,200,16,16,"1 error occurs in bit",BLACK);
						  char temp = error_position + '0';
						  char ep[2] = {temp,'\0'};
						  lcd_show_string(6*lcddev.width/7,3*lcddev.height/5+26,100,24,24,ep,BLACK);
						  char result[strlen("The error data is ")+strlen(wr_data)];
						  strcpy(result,"The error data is ");strcat(result,wr_data);
						  lcd_show_string(10,3*lcddev.height/5+50,200,16,16,result,BLACK);
					  }
					  else{
						  lcd_show_string(10,3*lcddev.height/5+30,200,16,16,"No errors",BLACK);
					  }
				  }
				  flag = 1;
			  }
		  }
	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //按键扫描
{
	if(htim == &htim2)
	{
		Key_Scan(button);
		Key_Scan(button+1);
		Key_Scan(button+2);
	}
}

// 计算冗余位的数量
uint8_t calculate_redundant_bits(uint8_t input_len, uint8_t mode) {
    uint8_t r = 0;
    if(mode == ENCODING){
    	while ((1 << r) < (input_len + r + 1)) {
    		r++;
    	}
    }
    else{
    	while ((1 << r) < input_len) {
    	        r++;
    	    }
    }
    return r;
}

uint8_t char_to_bit(char c) {
    return c == '1' ? 1 : 0;
}

char bit_to_char(uint8_t bit) {
    return bit ? '1' : '0';
}

// Hamming编码函数，输入为字符串
char* HAMMING_ENCODE(char* input) {
    uint8_t input_len = strlen(input);

    uint8_t r = calculate_redundant_bits(input_len,ENCODING);
    uint8_t total_len = input_len + r;

    // 分配空间存储编码后的数据
    char* encoded = (char*)malloc((total_len + 1) * sizeof(char));
    encoded[total_len] = '\0';

    // 初始化编码数组
    for (uint8_t i = 0; i < total_len; i++) {
        encoded[i] = '0';
    }

    uint8_t data_index = 0;
    for (uint8_t i = 1; i <= total_len; i++) {
        if ((i & (i - 1)) != 0) {
            encoded[i - 1] = input[data_index];
            data_index++;
        }
    }

    for (uint8_t i = 0; i < r; i++) {
        uint8_t pos = (1 << i);
        uint8_t parity = 0;

        for (uint8_t j = 1; j <= total_len; j++) {
            if (j & pos) {
                parity ^= char_to_bit(encoded[j - 1]);
            }
        }
        encoded[pos - 1] = bit_to_char(parity);
    }

    return encoded;
}

char* HAMMING_DECODE(char* hamming_input) {
    uint8_t input_len = strlen(hamming_input);


    uint8_t r = calculate_redundant_bits(input_len,DECODING);
    uint8_t error_pos = 0;
    for (uint8_t i = 0; i < r; i++) {
            uint8_t pos = (1 << i);
            uint8_t parity = 0;

            for (uint8_t j = 1; j <= input_len; j++) {
                if (j & pos) {
                    parity ^= char_to_bit(hamming_input[j - 1]);
                }
            }
            parity_val[r-i-1] = bit_to_char(parity);

            if (parity != 0) {
                error_pos += pos;
            }
        }
        parity_val[r] = '\0';

        if (error_pos > 0 && error_pos <= input_len) {
        	error_position = error_pos;
            hamming_input[error_pos - 1] = bit_to_char(!char_to_bit(hamming_input[error_pos - 1]));
        }

        uint8_t data_len = input_len - r;
        char* decoded = (char*)malloc((data_len + 1) * sizeof(char));

        decoded[data_len] = '\0';
        wr_data[data_len] = '\0';

        uint8_t data_index = 0;
        uint8_t di = 0;
        for (uint8_t i = 1; i <= input_len; i++) {
            if ((i & (i - 1)) != 0) {
                decoded[data_index++] = hamming_input[i - 1];
                if(i==error_pos){wr_data[di++]=bit_to_char(!char_to_bit(hamming_input[i-1]));}
                else{
                wr_data[di++] = hamming_input[i-1];
                }
            }
        }

        return decoded;
    }


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
