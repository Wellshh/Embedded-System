/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sys.h"
#include "delay.h"
//#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
//#include "usmart.h"
#include "touch.h"
#include "24cxx.h"
#include "24l01.h" //通信驱动 基于spi进行通信
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
//#include "remote.h" 红外遥控驱动
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 玩家形状尺寸
#define PLAYER_CIRCLE_RADIUS 20
#define PLAYER_RECT_WIDTH 40
#define PLAYER_RECT_HEIGHT 30
#define PLAYER_TRIANGLE_SIZE 40 // 假设为等边三角形，高度为40

// 障碍物形状尺寸
#define OBSTACLE_RECT_WIDTH 40
#define OBSTACLE_RECT_HEIGHT 30
#define OBSTACLE_CIRCLE_RADIUS 20
#define OBSTACLE_TRIANGLE_SIZE 40 // 假设为等边三角形，高度为40
#define MAX_OBSTACLES 4
typedef enum {
    SCENE_RUNNING,
    SCENE_SWIMMING,
    SCENE_ROWING,
    SCENE_MINING,
    SCENE_COUNT // 场景总数
} SceneType;

typedef enum {
    SHAPE_CIRCLE = 0,
    SHAPE_TRIANGLE,
    SHAPE_RECTANGLE,
    SHAPE_JUMP_ELLIPSE,      // 3
    SHAPE_JUMP_TRIANGLE_REVERSE, // 4
    SHAPE_JUMP_RECTANGLE_SMALL    // 5
} ShapeType;

ShapeType shape = SHAPE_CIRCLE;




// 当前场景
SceneType current_scene = SCENE_RUNNING;
int hit(int x, int y, u16 x_coordinate, u16 y_coordinate, ShapeType shape, int obstacle_shape);
typedef struct {
    u16 x;
    u16 y;
    u8 type; // 障碍物类型，根据场景不同可能有不同的表现
} Obstacle;

typedef enum {
    PLAYER_STATE_IDLE = 0,
    PLAYER_STATE_JUMPING,
    // 其他状态...
} PlayerState;
PlayerState player_state = PLAYER_STATE_IDLE;
Obstacle obstacles[MAX_OBSTACLES];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
unsigned char DATA_TO_SEND[800];
int state_num = 0;
u8 STATE[30];
UART_HandleTypeDef huart1;
int speed = 0;                 // 跑步速度
int initial_speed = 5;
u8 counter = 0;
//int x1,x2,y1,y2; //障碍物坐标
u16 background_color = WHITE;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void LCD_Draw_Shape(u16 x, u16 y, ShapeType shape) {
    switch(shape) {
        case SHAPE_CIRCLE:
            LCD_Draw_Circle(x, y, PLAYER_CIRCLE_RADIUS);
            break;
        case SHAPE_TRIANGLE:
            LCD_Draw_Triangle(x - 20, y + 20, x, y - 20, x + 20, y + 20);
            break;
        case SHAPE_RECTANGLE:
            LCD_DrawRectangle(x - 20, y - 15, x + 20, y + 15);
            break;
        case SHAPE_JUMP_ELLIPSE:
            LCD_Draw_Ellipse(x, y, 30, 15); // 确保有此函数
            break;
        case SHAPE_JUMP_TRIANGLE_REVERSE:
            LCD_Draw_Triangle(x - 20, y - 20, x, y + 20, x + 20, y - 20);
            break;
        case SHAPE_JUMP_RECTANGLE_SMALL:
            LCD_DrawRectangle(x - 10, y - 7, x + 10, y + 7);
            break;
        default:
            break;
    }
}

void LCD_Clear_Shape(u16 x, u16 y, u16 background_color, ShapeType shape) {
    POINT_COLOR = background_color;
    LCD_Draw_Shape(x, y, shape);
}

u16 perform_jump(u16 x, u16 y, ShapeType original_shape, ShapeType jump_shape, u16 background_color) {
    // 清除原始形状
    LCD_Clear_Shape(x, y, background_color, original_shape);
    y-=10;

    // 绘制跳跃形状
    POINT_COLOR = RED;
    LCD_Draw_Shape(x, y, jump_shape);

    // 延迟
    HAL_Delay(300);

    // 清除跳跃形状
    LCD_Clear_Shape(x, y, background_color, jump_shape);
    POINT_COLOR = RED;

    // 恢复原始形状
    LCD_Draw_Shape(x, y, original_shape);
    HAL_Delay(300);
    return y;
}

// 修改后的跳跃分支
u16 handle_jump(u16 x_coordinate, u16 y_coordinate, ShapeType current_shape, SceneType current_scene) {
    // 跳跃
    LED1 = !LED1;

    ShapeType jump_shape = SHAPE_CIRCLE; // 默认值

    // 根据当前场景和形状选择跳跃时的形状
    switch(current_scene) {
        case SCENE_RUNNING:
            jump_shape = SHAPE_JUMP_ELLIPSE;
            break;
        case SCENE_SWIMMING:
            // 例如，游泳场景的跳跃形状
        	jump_shape = SHAPE_JUMP_ELLIPSE;
            break;
        case SCENE_ROWING:
            // 例如，划船场景的跳跃形状
            // jump_shape = SHAPE_JUMP_ROWING; // 需要定义
        	jump_shape = SHAPE_JUMP_TRIANGLE_REVERSE;
            break;
        case SCENE_MINING:
            // 例如，采矿场景的跳跃形状
            // jump_shape = SHAPE_JUMP_MINING; // 需要定义
        	jump_shape = SHAPE_JUMP_RECTANGLE_SMALL;
            break;
        default:
            jump_shape = current_shape; // 默认不变
            break;
    }

    // 执行跳跃动画

    // 调整y坐标
    if(y_coordinate - 30 > 20){
        // 检查是否撞到障碍物
        for(int i = 0; i < MAX_OBSTACLES; i++) {
            if(hit(obstacles[i].x, obstacles[i].y, x_coordinate, y_coordinate - 90, current_shape, obstacles[i].type)) {
                // 过晚跳跃会撞到障碍物
                speed = 0;
                LCD_Clear(background_color);
                LCD_ShowString(lcddev.width / 2 - 150, lcddev.height / 2, 300, 24, 24, "You are dead!");
                HAL_Delay(1000);
                LCD_Clear(background_color);
                LCD_DisplayOff();
                return -1; // 发生碰撞后退出循环
            }
        }
        u16 temp = y_coordinate;
        y_coordinate -= 90;
        while(temp > y_coordinate){
        	temp = perform_jump(x_coordinate, temp, current_shape, jump_shape, background_color);
        }
        LCD_Clear(background_color);
    }
    return y_coordinate;
}

void init_random() {
    srand(HAL_GetTick());
}
static inline float clampf(float value, float min, float max)
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}
static inline float distance_squared(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}
/**
 * @brief 检查点是否在三角形内
 * @param px 点的X坐标
 * @param py 点的Y坐标
 * @param x0 三角形第一个顶点的X坐标
 * @param y0 三角形第一个顶点的Y坐标
 * @param x1 三角形第二个顶点的X坐标
 * @param y1 三角形第二个顶点的Y坐标
 * @param x2 三角形第三个顶点的X坐标
 * @param y2 三角形第三个顶点的Y坐标
 * @return 1表示点在三角形内，0表示不在
 */
int point_in_triangle(float px, float py, float x0, float y0, float x1, float y1, float x2, float y2)
{
    float denominator = ((y1 - y2)*(x0 - x2) + (x2 - x1)*(y0 - y2));
    if (denominator == 0) return 0; // 三角形退化

    float a = ((y1 - y2)*(px - x2) + (x2 - x1)*(py - y2)) / denominator;
    float b = ((y2 - y0)*(px - x2) + (x0 - x2)*(py - y2)) / denominator;
    float c = 1.0f - a - b;

    return (a >= 0) && (b >= 0) && (c >= 0);
}

/**
 * @brief 计算点到线段的最短距离
 * @param px 点的X坐标
 * @param py 点的Y坐标
 * @param x1 线段起点的X坐标
 * @param y1 线段起点的Y坐标
 * @param x2 线段终点的X坐标
 * @param y2 线段终点的Y坐标
 * @return 点到线段的距离
 */
float distance_point_to_segment(float px, float py, float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    if(dx == 0 && dy == 0) {
        // 线段退化为一个点
        return sqrtf(distance_squared(px, py, x1, y1));
    }

    float t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
    t = clampf(t, 0.0f, 1.0f);

    float closest_x = x1 + t * dx;
    float closest_y = y1 + t * dy;

    return sqrtf(distance_squared(px, py, closest_x, closest_y));
}
/**
 * @brief 检查圆形与矩形是否碰撞
 * @param cx 圆心X坐标
 * @param cy 圆心Y坐标
 * @param radius 圆的半径
 * @param rect_center_x 矩形中心X坐标
 * @param rect_center_y 矩形中心Y坐标
 * @param rect_width 矩形宽度
 * @param rect_height 矩形高度
 * @return 1表示碰撞，0表示不碰撞
 */
int circle_rectangle_collision(float cx, float cy, float radius, float rect_center_x, float rect_center_y, float rect_width, float rect_height)
{
    float rect_x = rect_center_x - rect_width / 2;
    float rect_y = rect_center_y - rect_height / 2;

    float closest_x = clampf(cx, rect_x, rect_x + rect_width);
    float closest_y = clampf(cy, rect_y, rect_y + rect_height);

    float dist_sq = distance_squared(cx, cy, closest_x, closest_y);

    return (dist_sq <= (radius * radius));
}
/**
 * @brief 检查两个圆形是否碰撞
 * @param cx1 第一个圆心X坐标
 * @param cy1 第一个圆心Y坐标
 * @param r1 第一个圆的半径
 * @param cx2 第二个圆心X坐标
 * @param cy2 第二个圆心Y坐标
 * @param r2 第二个圆的半径
 * @return 1表示碰撞，0表示不碰撞
 */
int circle_circle_collision(float cx1, float cy1, float r1, float cx2, float cy2, float r2)
{
    float dist_sq = distance_squared(cx1, cy1, cx2, cy2);
    float radii_sum_sq = (r1 + r2) * (r1 + r2);
    return (dist_sq <= radii_sum_sq);
}
/**
 * @brief 检查圆形与三角形是否碰撞
 * @param cx 圆心X坐标
 * @param cy 圆心Y坐标
 * @param radius 圆的半径
 * @param tx 三角形中心X坐标
 * @param ty 三角形中心Y坐标
 * @param size 三角形尺寸（假设为等边三角形，高度）
 * @return 1表示碰撞，0表示不碰撞
 */
int circle_triangle_collision(float cx, float cy, float radius, float tx, float ty, float size)
{
    // 计算三角形的三个顶点坐标（假设为等边三角形，顶点朝上）
    float h = size; // 高度
    float half_base = size / 2.0f;

    float x0 = tx;
    float y0 = ty - (2.0f / 3.0f) * h;

    float x1 = tx - half_base;
    float y1 = ty + (1.0f / 3.0f) * h;

    float x2 = tx + half_base;
    float y2 = ty + (1.0f / 3.0f) * h;

    // 检查圆心是否在三角形内
    if(point_in_triangle(cx, cy, x0, y0, x1, y1, x2, y2)) return 1;

    // 检查圆是否与三角形的任意一条边相交
    if(distance_point_to_segment(cx, cy, x0, y0, x1, y1) <= radius) return 1;
    if(distance_point_to_segment(cx, cy, x1, y1, x2, y2) <= radius) return 1;
    if(distance_point_to_segment(cx, cy, x2, y2, x0, y0) <= radius) return 1;

    return 0;
}
/**
 * @brief 检查三角形与矩形是否碰撞
 * @param tx 三角形中心X坐标
 * @param ty 三角形中心Y坐标
 * @param size 三角形尺寸（假设为等边三角形，高度）
 * @param rx 矩形中心X坐标
 * @param ry 矩形中心Y坐标
 * @param rw 矩形宽度
 * @param rh 矩形高度
 * @return 1表示碰撞，0表示不碰撞
 */
int triangle_rectangle_collision(float tx, float ty, float size, float rx, float ry, float rw, float rh)
{
    // 计算三角形的三个顶点坐标（假设为等边三角形，顶点朝上）
    float h = size; // 高度
    float half_base = size / 2.0f;

    float x0 = tx;
    float y0 = ty - (2.0f / 3.0f) * h;

    float x1 = tx - half_base;
    float y1 = ty + (1.0f / 3.0f) * h;

    float x2 = tx + half_base;
    float y2 = ty + (1.0f / 3.0f) * h;

    // 矩形的四个顶点
    float rect_x_min = rx - rw / 2.0f;
    float rect_y_min = ry - rh / 2.0f;
    float rect_x_max = rx + rw / 2.0f;
    float rect_y_max = ry + rh / 2.0f;

    // 检查三角形的任意一个顶点是否在矩形内
    if( (x0 >= rect_x_min && x0 <= rect_x_max && y0 >= rect_y_min && y0 <= rect_y_max) ||
        (x1 >= rect_x_min && x1 <= rect_x_max && y1 >= rect_y_min && y1 <= rect_y_max) ||
        (x2 >= rect_x_min && x2 <= rect_x_max && y2 >= rect_y_min && y2 <= rect_y_max) )
        return 1;

    // 检查矩形的任意一个顶点是否在三角形内
    if(point_in_triangle(rect_x_min, rect_y_min, x0, y0, x1, y1, x2, y2)) return 1;
    if(point_in_triangle(rect_x_max, rect_y_min, x0, y0, x1, y1, x2, y2)) return 1;
    if(point_in_triangle(rect_x_max, rect_y_max, x0, y0, x1, y1, x2, y2)) return 1;
    if(point_in_triangle(rect_x_min, rect_y_max, x0, y0, x1, y1, x2, y2)) return 1;

    // 可选：检查三角形的边是否与矩形的边相交
    // 为简化，暂不实现

    return 0;
}
/**
 * @brief 检查两个三角形是否碰撞
 * @param tx1 第一个三角形的中心X坐标
 * @param ty1 第一个三角形的中心Y坐标
 * @param size1 第一个三角形的尺寸（假设为等边三角形，高度）
 * @param tx2 第二个三角形的中心X坐标
 * @param ty2 第二个三角形的中心Y坐标
 * @param size2 第二个三角形的尺寸（假设为等边三角形，高度）
 * @return 1表示碰撞，0表示不碰撞
 */
int triangle_triangle_collision(float tx1, float ty1, float size1, float tx2, float ty2, float size2)
{
    // 计算第一个三角形的三个顶点坐标（假设为等边三角形，顶点朝上）
    float h1 = size1;
    float half_base1 = size1 / 2.0f;

    float x0_1 = tx1;
    float y0_1 = ty1 - (2.0f / 3.0f) * h1;

    float x1_1 = tx1 - half_base1;
    float y1_1 = ty1 + (1.0f / 3.0f) * h1;

    float x2_1 = tx1 + half_base1;
    float y2_1 = ty1 + (1.0f / 3.0f) * h1;

    // 计算第二个三角形的三个顶点坐标（假设为等边三角形，顶点朝上）
    float h2 = size2;
    float half_base2 = size2 / 2.0f;

    float x0_2 = tx2;
    float y0_2 = ty2 - (2.0f / 3.0f) * h2;

    float x1_2 = tx2 - half_base2;
    float y1_2 = ty2 + (1.0f / 3.0f) * h2;

    float x2_2 = tx2 + half_base2;
    float y2_2 = ty2 + (1.0f / 3.0f) * h2;

    // 检查第一个三角形的任意一个顶点是否在第二个三角形内
    if(point_in_triangle(x0_1, y0_1, x0_2, y0_2, x1_2, y1_2, x2_2, y2_2)) return 1;
    if(point_in_triangle(x1_1, y1_1, x0_2, y0_2, x1_2, y1_2, x2_2, y2_2)) return 1;
    if(point_in_triangle(x2_1, y2_1, x0_2, y0_2, x1_2, y1_2, x2_2, y2_2)) return 1;

    // 检查第二个三角形的任意一个顶点是否在第一个三角形内
    if(point_in_triangle(x0_2, y0_2, x0_1, y0_1, x1_1, y1_1, x2_1, y2_1)) return 1;
    if(point_in_triangle(x1_2, y1_2, x0_1, y0_1, x1_1, y1_1, x2_1, y2_1)) return 1;
    if(point_in_triangle(x2_2, y2_2, x0_1, y0_1, x1_1, y1_1, x2_1, y2_1)) return 1;

    // 可选：检查两个三角形的边是否相交
    // 为简化，暂不实现

    return 0;
}


void generate_obstacles() {
    for(int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].x = rand() % (lcddev.width - 40) + 20; // 确保障碍物在屏幕内
        obstacles[i].y = rand() % (lcddev.height - 100) + 50; // 避免生成在玩家初始位置
        obstacles[i].type = rand() % 3; // 根据需要定义不同类型的障碍物
    }
}

void draw_obstacles() {
    for(int i = 0; i < MAX_OBSTACLES; i++) {
        if(obstacles[i].type == 0) {
            // 绘制矩形障碍物
            LCD_DrawRectangle(obstacles[i].x, obstacles[i].y, obstacles[i].x + 40, obstacles[i].y + 30);
        }
        else if(obstacles[i].type == 1) {
            // 绘制圆形障碍物
            LCD_Draw_Circle(obstacles[i].x, obstacles[i].y, 20);
        }
        else if(obstacles[i].type == 2) {
            // 绘制三角形障碍物（假设有此函数）
            // 需要根据具体的LCD库实现绘制三角形
             LCD_Draw_Triangle(obstacles[i].x-20,obstacles[i].y+20,obstacles[i].x,obstacles[i].y-20,obstacles[i].x+20,obstacles[i].y+20);
        }
    }
}

void draw_scene_background() {
    switch(current_scene) {
        case SCENE_RUNNING:
            // 绘制跑步场景背景
        	background_color = WHITE;
        	shape = SHAPE_CIRCLE;
            LCD_Clear(WHITE);
            // 可以添加更多跑步场景的元素
            break;
        case SCENE_SWIMMING:
            // 绘制游泳场景背景
        	background_color = CYAN;
            LCD_Clear(CYAN);
            shape = SHAPE_CIRCLE;
            // 可以添加更多游泳场景的元素
            break;
        case SCENE_ROWING:
            // 绘制划船场景背景
        	background_color = BLUE;
            LCD_Clear(BLUE);
            shape = SHAPE_TRIANGLE;
            // 可以添加更多划船场景的元素
            break;
        case SCENE_MINING:
            // 绘制采矿场景背景
        	background_color = GRAY;
            LCD_Clear(GRAY);
            shape = SHAPE_RECTANGLE;
            // 可以添加更多采矿场景的元素
            break;
        default:
        	background_color = WHITE;
            LCD_Clear(WHITE);
            shape = SHAPE_CIRCLE;
            break;
    }
}
void switch_scene() {
    current_scene = (current_scene + 1) % SCENE_COUNT;
    draw_scene_background();
    generate_obstacles();
}
//清空屏幕并在右上角显�???"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏
 	POINT_COLOR=BLUE;//设置字体为蓝�???
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色
}
////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部�???
//画水平线
//x0,y0:坐标
//len:线长�???
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++)
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside
		{
 			if (x>imax)
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}
//两个数之差的绝对�???
//x1,x2：需取差值的两个�???
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{
	if(x1>x2)return x1-x2;
	else return x2-x1;
}
//画一条粗�???
//(x1,y1),(x2,y2):线条的起始坐�???
//size：线条的粗细程度
//color：线条的颜色

void screen_print(){
	LCD_Clear(WHITE);//清屏
	POINT_COLOR=BLUE;//设置字体为蓝�??
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
	LCD_ShowString(0,0,200,24,24, "SHOW PICTURE");
	LCD_ShowString(60,60,200,24,24, "SEND MESSAGE");
	LCD_ShowString(0, lcddev.height-24, 200, 16, 16, STATE);
	LCD_ShowString(0,110,200,24,24,"POKEMON GO");
	POINT_COLOR=RED;//设置画笔为红�??

}

void screen_draw_track(){
    // 根据当前场景绘制不同的跑道
    for (int i = 0; i <= lcddev.height - 50; i += 50){
        if(current_scene == SCENE_RUNNING) {
            POINT_COLOR = BLACK;
        }
        else if(current_scene == SCENE_SWIMMING) {
            POINT_COLOR = BLUE;
        }
        else if(current_scene == SCENE_ROWING) {
            POINT_COLOR = GREEN;
        }
        else if(current_scene == SCENE_MINING) {
            POINT_COLOR = BROWN;
        }
        LCD_DrawLine(lcddev.width/2, i, lcddev.width/2, i + 30);
    }
}




void screen_norm_print(u16 x, u16 y){
    LCD_Clear_Rectangle(lcddev.width-150,30,200,16,background_color);
    LCD_Clear_Rectangle(lcddev.width-150,50,200,16,background_color);
    char speed_info[80];
    sprintf(speed_info, "The speed is %d", speed);
    POINT_COLOR = BLUE;
    LCD_ShowString(lcddev.width-24,0,200,8,8,"RST"); //显示清屏区域
    char time_info[80];
    sprintf(time_info, "The time is %d",counter);
    screen_draw_track();
    draw_obstacles(); // 绘制障碍物
    POINT_COLOR = RED;
    // 根据当前场景绘制不同形状的玩家
    if(current_scene == SCENE_RUNNING || current_scene == SCENE_SWIMMING) {
        LCD_Draw_Circle(x, y, 20); // 选手
    }
    else if(current_scene == SCENE_ROWING) {
        // 绘制划船的形状，例如一个椭圆
    	LCD_Draw_Triangle(x - 20, y + 20, x, y - 20, x + 20, y + 20);
    }
    else if(current_scene == SCENE_MINING) {
        // 绘制采矿的形状，例如一个方块
        LCD_DrawRectangle(x - 20, y - 20, x + 20, y + 20);
    }
    LCD_ShowString(lcddev.width-150,30,200,16,16,speed_info); //显示当前速度
    LCD_ShowString(lcddev.width-150,70,200,16,16,time_info); //显示时间
    char initial_speed_info[80];
    sprintf(initial_speed_info, "Initial speed is %d", initial_speed);
    LCD_ShowString(lcddev.width-150,50,200,16,16,initial_speed_info); //显示初速度
    //画互动按钮
    LCD_ShowString(lcddev.width-40, lcddev.height-20, 200, 16, 16, "Jump");
    LCD_ShowString(lcddev.width-40, lcddev.height-100, 200, 16, 16, "Right");
    LCD_ShowString(lcddev.width-40, lcddev.height-180, 200, 16, 16, "Left");
}

void change_state(){
	if(state_num == 0){
		state_num = 1;
		sprintf(STATE, "STATE: ON");
	}else{
		state_num = 0;
		sprintf(STATE, "STATE: OFF");
	}
}

void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	if(x1<size|| x2<size||y1<size|| y2<size)return;
	delta_x=x2-x1; //计算坐标增量
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向
	else if(delta_x==0)incx=0;//垂直�???
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;//水平�???
	else{incy=-1;delta_y=-delta_y;}
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标�???
	else distance=delta_y;
	for(t=0;t<=distance+1;t++ )//画线输出
	{
		gui_fill_circle(uRow,uCol,size,color);//画点
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
//5个触控点的颜�???(电容触摸屏用)
const u16 POINT_COLOR_TBL[5]={RED,GREEN,BLUE,BROWN,GRED};
int max(int a,int b){
	return a > b ? a : b;
}
int min(int a, int b){
	return a < b ? a : b;
}
//int hit(int x, int y, u16 x_coordinate, u16 y_coordinate){
//	return ((x_coordinate >= (x - 20) && y_coordinate >= (y - 20)) && (x_coordinate <= (x + 40 + 20) && (y_coordinate <= (y + 30 + 20))));
//}
/**
 * @brief 检查玩家与障碍物是否碰撞
 * @param x 障碍物中心X坐标
 * @param y 障碍物中心Y坐标
 * @param x_coordinate 玩家中心X坐标
 * @param y_coordinate 玩家中心Y坐标
 * @param shape 玩家形状（0: 圆形, 1: 三角形, 2: 矩形）
 * @param obstacle_shape 障碍物形状（0: 矩形, 1: 圆形, 2: 三角形）
 * @return 1表示碰撞，0表示不碰撞
 */
int hit(int x, int y, u16 x_coordinate, u16 y_coordinate, ShapeType shape, int obstacle_shape)
{
    if(shape == SHAPE_CIRCLE) { // 玩家是圆形
        if(obstacle_shape == 0) { // 障碍物是矩形
            // 玩家圆形
            return circle_rectangle_collision(
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_CIRCLE_RADIUS,
                (float)x,
                (float)y,
                OBSTACLE_RECT_WIDTH,
                OBSTACLE_RECT_HEIGHT
            );
        }
        else if(obstacle_shape == 1) { // 障碍物是圆形
            // 两个圆形
            return circle_circle_collision(
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_CIRCLE_RADIUS,
                (float)x,
                (float)y,
                OBSTACLE_CIRCLE_RADIUS
            );
        }
        else if(obstacle_shape == 2) { // 障碍物是三角形
            // 玩家圆形与障碍物三角形
            return circle_triangle_collision(
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_CIRCLE_RADIUS,
                (float)x,
                (float)y,
                OBSTACLE_TRIANGLE_SIZE
            );
        }
    }
    else if(shape == SHAPE_TRIANGLE) { // 玩家是三角形
        if(obstacle_shape == 0) { // 障碍物是矩形
            // 玩家三角形与障碍物矩形
            return triangle_rectangle_collision(
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_TRIANGLE_SIZE,
                (float)x,
                (float)y,
                OBSTACLE_RECT_WIDTH,
                OBSTACLE_RECT_HEIGHT
            );
        }
        else if(obstacle_shape == 1) { // 障碍物是圆形
            // 障碍物圆形与玩家三角形
            return circle_triangle_collision(
                (float)x,
                (float)y,
                OBSTACLE_CIRCLE_RADIUS,
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_TRIANGLE_SIZE
            );
        }
        else if(obstacle_shape == 2) { // 障碍物是三角形
            // 两个三角形
            return triangle_triangle_collision(
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_TRIANGLE_SIZE,
                (float)x,
                (float)y,
                OBSTACLE_TRIANGLE_SIZE
            );
        }
    }
    else if(shape == SHAPE_RECTANGLE) { // 玩家是矩形
        if(obstacle_shape == 0) { // 障碍物是矩形
            // 玩家矩形与障碍物矩形（AABB碰撞）
            float player_left = (float)x_coordinate - PLAYER_RECT_WIDTH / 2.0f;
            float player_right = (float)x_coordinate + PLAYER_RECT_WIDTH / 2.0f;
            float player_top = (float)y_coordinate - PLAYER_RECT_HEIGHT / 2.0f;
            float player_bottom = (float)y_coordinate + PLAYER_RECT_HEIGHT / 2.0f;

            float obstacle_left = (float)x - OBSTACLE_RECT_WIDTH / 2.0f;
            float obstacle_right = (float)x + OBSTACLE_RECT_WIDTH / 2.0f;
            float obstacle_top = (float)y - OBSTACLE_RECT_HEIGHT / 2.0f;
            float obstacle_bottom = (float)y + OBSTACLE_RECT_HEIGHT / 2.0f;

            if(player_right < obstacle_left || player_left > obstacle_right ||
               player_bottom < obstacle_top || player_top > obstacle_bottom)
                return 0;
            else
                return 1;
        }
        else if(obstacle_shape == 1) { // 障碍物是圆形
            // 玩家矩形与障碍物圆形
            return circle_rectangle_collision(
                (float)x,
                (float)y,
                OBSTACLE_CIRCLE_RADIUS,
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_RECT_WIDTH,
                PLAYER_RECT_HEIGHT
            );
        }
        else if(obstacle_shape == 2) { // 障碍物是三角形
            // 玩家矩形与障碍物三角形
            return triangle_rectangle_collision(
                (float)x,
                (float)y,
                OBSTACLE_TRIANGLE_SIZE,
                (float)x_coordinate,
                (float)y_coordinate,
                PLAYER_RECT_WIDTH,
                PLAYER_RECT_HEIGHT
            );
        }
    }

    return 0; // 默认不碰撞
}

void rtp_test(void)
{
	u8 key;
	u8 i=0;
	u8 flag = 0; //表示刚刚结束滑动
	u16 last_x = 0, last_y = 0;   // 上次触摸�?
	u32 stop_time = 0;             // 上次滑动的时�?
	int inertia = 1;               // 惯�?��?�度
	u32 start_time = 0;
	u16 x_coordinate = lcddev.width/2;
	u16 y_coordinate = lcddev.height - 30;
	u32 time_slot[2] = {0,0};
	u8 bit = 0;
	u8 escape_flag = 0;
	u8 escape_lock = 0;

	u32 race_start_time = HAL_GetTick();
	const u32 race_time_limit = 60000; // 比赛时间限制，单位毫秒（60秒）

	    // 初始化随机数
	init_random();
	generate_obstacles();

	draw_scene_background();
	while(1)
	{
	 	key=KEY_Scan(0);
		tp_dev.scan(0);
		screen_norm_print(x_coordinate,y_coordinate);
		if(key == KEY1_PRES) { // 按下KEY1切换场景
		            switch_scene();
		            // 重置玩家位置和速度
		            x_coordinate = lcddev.width / 2;
		            y_coordinate = lcddev.height - 30;
		            speed = initial_speed;
		            race_start_time = HAL_GetTick(); // 重置比赛开始时间
		 }
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{
			escape_lock = 1;
			if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height){
			if (tp_dev.x[0] > lcddev.width - 45 && tp_dev.y[0] > lcddev.height - 25){
				//跳跃
				LED1 = !LED1;
				if(player_state == PLAYER_STATE_IDLE){
					player_state = PLAYER_STATE_JUMPING;
					u16 temp = handle_jump(x_coordinate, y_coordinate, shape, current_scene);
					if(temp != -1){
						y_coordinate = temp;
					}
					player_state = PLAYER_STATE_IDLE;
				}
			}
			else if(tp_dev.x[0] > lcddev.width - 45 && tp_dev.y[0] > lcddev.height - 105 && tp_dev.y[0] <= lcddev.height - 25){
				//向右移动
				LED1 = !LED1;
				if(x_coordinate + 10 < lcddev.width - 20){
//					LCD_Clear_Circle(x_coordinate,y_coordinate,20,WHITE);
					LCD_Clear_Shape(x_coordinate,y_coordinate,background_color,shape);
					x_coordinate += 10;
				}
				else{
					//碰到边界，立即减速为0，提示需要转向
					speed = 0;
					LCD_ShowString(lcddev.width/2,lcddev.height/2,300,24,24,"TURN!");
					HAL_Delay(1000);
					LCD_Clear(background_color);
				}
			}
			else if(tp_dev.x[0] > lcddev.width - 45 && tp_dev.y[0] > lcddev.height - 185 && tp_dev.y[0] <= lcddev.height - 105){
				//向左移动
				LED1 = !LED1;
				if(x_coordinate - 10 > 20){
//					LCD_Clear_Circle(x_coordinate,y_coordinate,20,WHITE);
					LCD_Clear_Shape(x_coordinate,y_coordinate,background_color,shape);
					x_coordinate -= 10;
				}
				else{
					//碰到边界，立即减速为0，提示需要转向
					speed = 0;
					LCD_ShowString(lcddev.width/2,lcddev.height/2,300,24,24,"TURN!");
					HAL_Delay(1000);
					LCD_Clear(background_color);
				}
			}
			else if (tp_dev.x[0] <= lcddev.width - 45){
				if(flag){
					flag = 0;
					start_time = HAL_GetTick();
					time_slot[bit] = start_time - stop_time;
					bit = !bit;
					}
		 		if(last_x != 0 && last_y != 0){
		 			if(time_slot[1] > time_slot[0]){
		 				//频率降低，速度变小
		 				initial_speed = max(initial_speed - 1,1);
		 			}
		 			else{
		 				//频率增加，速度变大
		 				initial_speed = min(initial_speed + 1, 10);
		 			}
					if (tp_dev.y[0] < last_y){ //向上滑动：前�?
						speed = initial_speed; //初始速度
					}
					else if(tp_dev.y[0] > last_y){//向下滑动：后�?
						speed = -initial_speed;
					}
		 		}
				last_x = tp_dev.x[0];
				last_y = tp_dev.y[0];
			}
			}
		}
		else {
			if(escape_lock == 1){
				escape_flag = 0;
				escape_lock = 0;
			}
			if(!flag){
				flag = 1;
				stop_time = HAL_GetTick();
			}
			last_x = 0;
			last_y = 0;
			if (y_coordinate - speed < lcddev.height -30){
//						LCD_Clear_Circle(x_coordinate,y_coordinate,20,WHITE);
				LCD_Clear_Shape(x_coordinate,y_coordinate,background_color,shape);
						y_coordinate -= speed;
					}//移动
				 		//没有滑动时，慢慢减�??
				 		if(speed > 0){
				 			speed -= inertia;
				 			if(speed <0) speed = 0;
				 		}
				 		else if(speed <0){
				 			speed += inertia;
				 			if(speed >0) speed =0;
				 		}

			}

		for(int i = 0; i < MAX_OBSTACLES; i++) {
		            if(hit(obstacles[i].x, obstacles[i].y, x_coordinate, y_coordinate,shape,obstacles[i].type) && !escape_flag){
		                speed = 0;
		                escape_flag = 1; //等待触摸后将escape_flag重新置为0
		                escape_lock = 0;
		                LCD_Clear(background_color);
		                POINT_COLOR = RED;
		                LCD_ShowString(lcddev.width / 2, lcddev.height / 2, 300, 24, 24, "HIT!");
		                HAL_Delay(1000);
		                LCD_Clear(background_color);
		            }
		        }
		//完赛提示
		if (y_coordinate <= 20){
			u32 race_finish_time = HAL_GetTick();
			speed = 0;
			LCD_Clear(background_color);
			POINT_COLOR = RED;
			if (counter >= 30){
				LCD_ShowString(lcddev.width/2 - 100,lcddev.height/2,300,16,16,"Race is Finished out of time!");
			}
			else{
			LCD_ShowString(lcddev.width/2 - 100,lcddev.height/2,300,16,16,"Race is Finished in time!");
			}
			HAL_Delay(1000);
			LCD_Clear(background_color);
		}

		if(key==KEY0_PRES)	//KEY0按下,则执行校准程�???
		{
			LCD_Clear(WHITE);	//清屏
		    TP_Adjust();  		//屏幕校准
			TP_Save_Adjdata();
			Load_Drow_Dialog();
		}
		i++;
		if(i%15==0){LED0=!LED0; counter ++;};
	}
}
//电容触摸屏测试函�???
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;
 	u16 lastpos[5][2];		//�???后一次的数据
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<5;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
                //printf("X坐标:%d,Y坐标:%d\r\n",tp_dev.x[0],tp_dev.y[0]);
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}

					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<20)
					{
						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}

		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
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
  Stm32_Clock_Init(RCC_PLL_MUL9);   	//设置时钟,72M
	delay_init(72);               		//初始化延时函�???
//	uart_init(115200);					//初始化串�???
//	usmart_dev.init(84); 		  	  	//初始化USMART
	LED_Init();							//初始化LED
	KEY_Init();							//初始化按�???
	LCD_Init();							//初始化LCD
	tp_dev.init();				   		//触摸屏初始化
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  init_random();
  HAL_TIM_Base_Start_IT(&htim2);
  POINT_COLOR=RED;
//  	LCD_ShowString(30,50,200,16,16,"Mini STM32");
//  	LCD_ShowString(30,70,200,16,16,"TOUCH TEST");
//  	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
//  	LCD_ShowString(30,110,200,16,16,"2019/11/15");
     	if(tp_dev.touchtype!=0XFF)
  	{
  		LCD_ShowString(30,130,200,16,16,"Press KEY0 to Adjust");//电阻屏才显示
  	}
  	delay_ms(1500);
   	Load_Drow_Dialog();

  	if(tp_dev.touchtype&0X80)ctp_test();//电容�??
  	else rtp_test(); 					//电阻�??
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//void HAL_TIM_PeriodElaspedCallback(TIM_HandleTypeDef *htim){
//	if (htim == &htim2){
//		screen_norm_print(x_coordinate,y_coordinate);
//	}
//	LED1 = !LED1;
//}

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
