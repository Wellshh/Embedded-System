/*
 * KEY.c
 *
 *  Created on: Oct 17, 2024
 *      Author: Wells
 *      部分代码来自https://blog.csdn.net/weixin_56719449/article/details/136275388?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-136275388-blog-141367609.235^v43^control&spm=1001.2101.3001.4242.1&utm_relevant_index=3
 */


#include "KEY.h"
#include "stdio.h"

buttonType button[3];

void Key_Config()
{
	button[0].GPIO_Port = KEY0_GPIO_Port;
	button[0].GPIO_Pin = KEY0_Pin;

	button[1].GPIO_Port = KEY1_GPIO_Port;
	button[1].GPIO_Pin = KEY1_Pin;

	button[2].GPIO_Port = KEY_WK_GPIO_Port;
	button[2].GPIO_Pin = KEY_WK_Pin;

}

void Key_ParaInit(buttonType* button)
{
	button->high_cnt = 0;
	button->hold_cnt = 0;
	button->press_flag = 0;
	button->release_flag = 0;
}

void Key_Scan(buttonType* button)
{
	switch(button->key)
	{
		case KEY_NULL:
		{
			if (button->GPIO_Pin == KEY_WK_Pin){
				if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 1){
					button->key = KEY_DOWN;
				}
				else if(HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 0){
					button->key = KEY_NULL;
				}
			}
			else {
			/* if falling edge captured */
				if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 0)
				{
					button->key = KEY_DOWN;
				}
				else if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 1)
				{
					button->key = KEY_NULL;
				}
			}

			/* if button is released ,high_time_count++ */
			if(button->release_flag == 1)
			{
				button->high_cnt++;
			}

			/**********************judge***********************/
			/* if high_time_count is longer than LONG_PRESS_TIME, consider BUTTON_LONG_PRESS */
			if(button->hold_cnt > LONG_PRESS_TIME)
			{
				button->buttonAction = BUTTON_LONG_PRESS;
				Key_ParaInit(button);
			}
			/* if high_time_count is shorter than LONG_PRESS_TIME,but longer than CLICK_MAX_TIME consider INVALID */
			else if(button->hold_cnt < LONG_PRESS_TIME && button->hold_cnt > CLICK_MAX_TIME)
			{
				Key_ParaInit(button);
			}

			/*
				only the latest press time is in range of [CLICK_MIN_TIME,CLICK_MAX_TIME] can be regarded valid
				if high level time > JUDGE_TIME also means that over the JUDGE_TIME and still dont have button pushed
				we can check the flag value to get button state now
			*/
			else if((button->high_cnt > JUDGE_TIME)&&(button->hold_cnt > CLICK_MIN_TIME && button->hold_cnt < CLICK_MAX_TIME))
			{
				if(button->press_flag ==1)
				{
					button->buttonAction = BUTTON_SINGLE;
				}
				else if(button->press_flag == 2)
				{
					button->buttonAction = BUTTON_DOUBLE;
				}
				else if(button->press_flag == 3)
				{
					button->buttonAction = BUTTON_TRIPLE;
				}

				Key_ParaInit(button);
			}
			break;
		}

		case KEY_DOWN:
		{
			button->key = KEY_PRESS;

			/* as long as falling edge occurring,press_flag++ */
			button->press_flag++;

			button->release_flag = 0; 			/* means that the button has been pressed */

			button->hold_cnt = 0;				/* reset hold time count */
			break;
		}

		case KEY_PRESS:
		{
			/* when button was kept pressed, hold count++ */
			if(button->GPIO_Pin == KEY_WK_Pin){
				if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 1){
					button->key = KEY_PRESS;
					button->hold_cnt++;
				}
				else if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 0){
					button->key = KEY_UP;
				}
			}
			else{
				if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 0)
				{
					button->key = KEY_PRESS;
					button->hold_cnt++;
				}
			/* when button was released, change state */
				else if(HAL_GPIO_ReadPin(button->GPIO_Port,button->GPIO_Pin) == 1)
				{
					button->key = KEY_UP;
				}
			}
			break;
		}

		case KEY_UP:
		{
			button->key = KEY_NULL;

			button->release_flag = 1;			/* means that the button is released */

			button->high_cnt = 0;				/* reset hold time count */

			/* if press time is longer than 1s then press_flag-- */
			if(button->hold_cnt > 100)
			{
				button->press_flag--;
			}
			break;
		}
		default:
			break;
	}
}



//void Key_Debug()
//{
//
//	for(uint8_t i=0;i<3;i++){
//		switch(button[i].buttonAction)
//		{
//
//			case BUTTON_SINGLE:
//			{
//				button[i].buttonAction = BUTTON_NULL;
//				lcd_clear(WHITE); g_back_color = WHITE;
//				lcd_show_string(10,lcddev.height/3,200,24,24,"single",BLACK);
//				HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
//				break;
//			}
//			case BUTTON_LONG_PRESS:
//			{
////				button[i].buttonAction = BUTTON_NULL;
////				lcd_clear(WHITE); g_back_color = WHITE;
////				lcd_show_string(10,lcddev.height/3,200,24,24,"long",BLACK);
//				break;
//			}
//			case BUTTON_DOUBLE:
//			{
//				button[i].buttonAction = BUTTON_NULL;
//				lcd_clear(WHITE); g_back_color = WHITE;
//				lcd_show_string(10,lcddev.height/3,200,24,24,"double",BLACK);
//				break;
//			}
//			case BUTTON_TRIPLE:
//			{
//				button[i].buttonAction = BUTTON_NULL;
//				lcd_clear(WHITE); g_back_color = WHITE;
//				lcd_show_string(10,lcddev.height/3,200,24,24,"triple",BLACK);
//				break;
//			}
//			case BUTTON_NULL:
//			{
//				button[i].buttonAction = BUTTON_NULL;
//				break;
//			}
//			default:
//			{
//				break;
//			}
//		}
//	}
//
//}
