/*
 * lcd.c
 *
 *  Created on: Oct 17, 2024
 *      Author: Wells
 */

#include "lcd_v4.h"
#include "LCD.h"

void lcd_divide(){
	//three lines
	lcd_fill(0,lcddev.height/5, lcddev.width,lcddev.height/5,BLACK);
	lcd_fill(0,2 * lcddev.height/5, lcddev.width,2 * lcddev.height/5, BLACK);
	lcd_fill(0, 3 * lcddev.height/5, lcddev.width,3 * lcddev.height/5,  BLACK);
}

void lcd_picture_mode(uint8_t Mode){
	if(Mode){
		lcd_show_string(10,lcddev.height/10,200,24,24,"Encoding Mode",BLACK);
		lcd_show_string(10,lcddev.height/10 + 40,100,16,16,"Original data", BLACK);
		lcd_show_string(10,2*lcddev.height/5 + 10, 100, 16,16,"Hamming code", BLACK);
		lcd_show_string(10,4*lcddev.height/5,200,24,24,"No errors", BLACK);
	}
	else {
		lcd_show_string(10,lcddev.height/10,200,24,24,"Decoding Mode",BLACK);
		lcd_show_string(10,lcddev.height/10 + 40,100,16,16,"Original data", BLACK);
		lcd_show_string(10,2*lcddev.height/5 + 10, 100, 16,16,"Hamming code", BLACK);
	}
}


void lcd_clear_area(uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{//局部清除函数
    uint32_t index = 0;
    uint32_t totalpoint = width * height;   /* 计算需要清除的总像素点数 */

    lcd_set_cursor(x0, y0);
    lcd_write_ram_prepare();

    LCD_RS(1);
    LCD_CS(0);

    for (index = 0; index < totalpoint; index++)
    {
        LCD_DATA_OUT(color);
        LCD_WR(0);
        LCD_WR(1);
    }

    LCD_CS(1);
}
