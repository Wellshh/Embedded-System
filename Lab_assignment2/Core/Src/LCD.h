/*
 * lcd.h
 *
 *  Created on: Oct 17, 2024
 *      Author: Wells
 */

#ifndef SRC_LCD_H_
#define SRC_LCD_H_
#define ENCODING 1
#define DECODING 0


void lcd_divide();
void lcd_picture_mode(uint8_t Mode);
void lcd_clear_area(uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);


#endif /* SRC_LCD_H_ */
