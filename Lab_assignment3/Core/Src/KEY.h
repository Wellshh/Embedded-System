/*
 * KEY.h
 *
 *  Created on: Oct 17, 2024
 *      Author: Wells
 *
 * 部分代码来自CSDN https://blog.csdn.net/weixin_56719449/article/details/136275388?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-136275388-blog-141367609.235^v43^control&spm=1001.2101.3001.4242.1&utm_relevant_index=3
 */

#ifndef SRC_KEY_H_
#define SRC_KEY_H_


//#include "tim.h"
#include "main.h"
/*
double click:
```___________``````````````___________````````````
     min< <max    <judge      min< <max     >judge

single click:
``````___________`````````
       min< <max  >judge

*/

#define LONG_PRESS_TIME 	     100
#define CLICK_MIN_TIME 		5	/* if key press_cnt time less than this -> invalid click */
#define CLICK_MAX_TIME 		20	/* if key press_cnt time more than this -> invalid click */
#define JUDGE_TIME 			20	/* double click time space */


typedef enum
{
	KEY_NULL,
	KEY_DOWN,
	KEY_PRESS,
	KEY_UP,
}KeyActionType;

typedef enum
{
	BUTTON_NULL,
	BUTTON_SINGLE,
	BUTTON_DOUBLE,
	BUTTON_TRIPLE,
	BUTTON_LONG_PRESS,
}ButtonActionType;

typedef struct
{
	GPIO_TypeDef * GPIO_Port;		//按键端口
	uint16_t GPIO_Pin;				//按键PIN
	KeyActionType key;				//按键类型
	uint16_t hold_cnt;				//按压计数器
	uint16_t high_cnt;				//高电平计数器
	uint8_t press_flag;				//按压标志
	uint8_t release_flag;			//松手标志
	ButtonActionType buttonAction;	//按键键值
}buttonType;

extern buttonType button[3];

void Key_Scan(buttonType*);
void Key_Debug();
void Key_Config();



#endif /* SRC_KEY_H_ */
