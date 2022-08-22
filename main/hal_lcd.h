#pragma once
#include "font.h"
#include "st7735.h"
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include  "public.h"

//������ʾλ��
#define LCD_DATE_SHOW_X                         0
#define LCD_DATE_SHOW_Y                         55
//������ʾ��ɫ
#define LCD_DATE_SHOW_COLOR                     COLOR_WHITE
#define LCD_DATE_SHOW_BACK_COLOR                COLOR_BLACK

//ʱ��(����) - ��ʾλ��
#define LCD_TIME_SHOW_X                         56
#define LCD_TIME_SHOW_Y                         103
//ʱ��(����) - ��ʾ��ɫ
#define LCD_TIME_SHOW_COLOR                     COLOR_WHITE
#define LCD_TIME_SHOW_BACK_COLOR                COLOR_BLACK

//����λ��
#define LCD_LINE_X                              0
#define LCD_LINE_Y                              150
//�ߴ�
#define LCD_LINE_HEIGHT                         1
//������ɫ
#define LCD_LINE_COLOR                          COLOR_WHITE

//ʱ��(С��) - ��ʾλ��
#define  LCD_SMALL_TIME_SHOW_X                  0
#define  LCD_SMALL_TIME_SHOW_Y                  155
//ʱ��(С��) - ��ʾ��ɫ
#define  LCD_SMALL_TIME_SHOW_COLOR              COLOR_WHITE
#define  LCD_SMALL_TIME_SHOW_BACK_COLOR         COLOR_BLACK

//�¶�(С��) - ��ʾλ��
#define  LCD_TEMP_SMALL_SHOW_X                  199
#define  LCD_TEMP_SMALL_SHOW_Y                  155
//�¶�(С��) - ��ʾ��ɫ
#define  LCD_TEMP_SMALL_SHOW_COLOR              COLOR_WHITE
#define  LCD_TEMP_SMALL_SHOW_BACK_COLOR         COLOR_BLACK

//�¶�(����) - ��ʾλ��
#define  LCD_TEMP_SHOW_X                        120
#define  LCD_TEMP_SHOW_Y                        82
//�¶�(����) - ��ʾ��ɫ
#define  LCD_TEMP_SHOW_COLOR                    COLOR_WHITE
#define  LCD_TEMP_SHOW_BACK_COLOR               COLOR_BLACK

//����ͼ��ˢ��λ��
#define LCD_WEATHER_ICON_X                      56
#define LCD_WEATHER_ICON_Y                      70
//����ͼ���С
#define LCD_WEATHER_ICON_WIDTH                  56
#define LCD_WEATHER_ICON_HEIGHT                 56
//����ͼ����ɫ
#define LCD_WEATHER_ICON_COLOR                  COLOR_WHITE
#define LCD_WEATHER_ICON_BACK_COLOR             COLOR_BLACK

//ˢ����ʱ���ص�״̬
typedef enum
{
    LCD_DISPLAY_OK             = 0,      //��ʾ����
    LCD_DISPLAY_ADDRESS_ERR    = 1,      //��ʾ��ַ���
    LCD_DISPLAY_NOT_FOUND_ERR  = 2,      //δ�ҵ���ʾ������
    LCD_DISPLAY_OVERFLOW_ERR   = 4,      //ˢ������Խ��(�����Ļ)
    LCD_DISPLAY_AREA_ERR       = 5,      //���ֵ��������, ���绮��һ���߶�Ϊ0������

    LCD_DISPLAY_SLIDE_STEP_ERR = 6,      //�õ�Ƭ��������
    LCD_DISPLAY_SLIDE_DIR_ERR  = 7,      //�õ�Ƭ�������
}Lcd_Diplay_State_e;

//�õ�Ƭ�ƶ��ƶ�����
typedef enum _lcd_slide_move_direction
{
    Lcd_Slide_Top,
    Lcd_Slide_Bottom,
    Lcd_Slide_Left,
    Lcd_Slide_Right,
}Lcd_Slide_Dir_e;

//�õ�Ƭ����
typedef struct _lcd_slide_config
{
    uint16_t x;                //x��
    uint16_t y;                //y��
    uint16_t width;            //��, �õ�Ƭ������, 8�ı���
    uint16_t height;           //��, �õ�Ƭ������, 8�ı���
    
    Lcd_Slide_Dir_e slide_dir; //�õ�Ƭ�ƶ�����
    uint16_t speed_ms;         //�õ�Ƭ�ٶ�, ���ٺ����ƶ�һ����

    uint16_t src_color;        //��ǰ��ʾ�Ľ�������/ͼ����ɫ
    uint16_t src_back_color;   //��ǰ��ʾ����ı���ɫ
    uint8_t *src_buf;          //src_buf ��С������ width*height/8, ���������

    uint16_t dest_color;       //Ŀ����ʾ�Ľ�������/ͼ����ɫ
    uint16_t dest_back_color;  //Ŀ����ʾ����ı���ɫ
    uint8_t *dest_buf;         //dest_buf ��С������ width*height/8, ���������

    uint8_t step_len;          //����, һ���ƶ���������, ����ƶ���ʽ��top �� bottom, step_len < height����, ��������ƶ���ʽ��left �� right, step_len������8�ı���

}Lcd_Slide_Config_t;


//��֪���� - ͼ������
typedef enum _Weather_Icon_Index
{
    Weather_Icon_Index_Sunny = 0,                     //0 - �磨���ڳ��а����磩
    Weather_Icon_Index_Clear = 1,                     //1 - �磨���ڳ���ҹ���磩
    Weather_Icon_Index_Fair = 0,                      //2 - �磨������а����磩
    Weather_Icon_Index_Fair_Night = 3,                //3 - �磨�������ҹ���磩
    Weather_Icon_Index_Cloudy = 2,                    //4 - ����
    Weather_Icon_Index_Partly_Cloudy = 3,             //5 - ������ - ����
    Weather_Icon_Index_Partly_Cloudy_Night = 4,       //6 - ������ - ����
    Weather_Icon_Index_Mostly_Cloudy = 3,             //7 - �󲿶��� - ����
    Weather_Icon_Index_Mostly_Cloudy_Night = 4,       //8 - �󲿶��� - ����
    Weather_Icon_Index_Overcast = 5,                  //9 - ��
    Weather_Icon_Index_Shower = 6,                    //10 - ����
    Weather_Icon_Index_Thundershower = 7,             //11 - ������
    Weather_Icon_Index_Thundershower_with_Hail = 8,   //12 - ��������б���
    Weather_Icon_Index_Light_Rain = 9,                //13 - С��
    Weather_Icon_Index_Moderate_Rain = 10,            //14 - ����
    Weather_Icon_Index_Heavy_Rain = 11,               //15 - ����
    Weather_Icon_Index_Storm = 12,                    //16 - ����
    Weather_Icon_Index_Heavy_Storm = 12,              //17 - ����
    Weather_Icon_Index_Severe_Storm = 12,             //18 - �ش���
    Weather_Icon_Index_Ice_Rain = 10,                 //19 - ����
    Weather_Icon_Index_Sleet = 13,                    //20 - ���ѩ
    Weather_Icon_Index_Snow_Flurry = 14,              //21 - ��ѩ
    Weather_Icon_Index_Light_Snow = 15,               //22 - Сѩ
    Weather_Icon_Index_Moderate_Snow = 16,            //23 - ��ѩ
    Weather_Icon_Index_Heavy_Snow = 17,               //24 - ��ѩ
    Weather_Icon_Index_Snowstorm = 17,                //25 - ��ѩ
    Weather_Icon_Index_Dust = 18,                     //26 - ����
    Weather_Icon_Index_Sand = 18,                     //27 - ��ɳ
    Weather_Icon_Index_Duststorm = 19,                //28 - ɳ����
    Weather_Icon_Index_Sandstorm = 19,                //29 - ǿɳ����
    Weather_Icon_Index_Foggy = 20,                    //30 - ��
    Weather_Icon_Index_Haze = 21,                     //31 - ��
    Weather_Icon_Index_Windy = 22,                    //32 - ��
    Weather_Icon_Index_Blustery = 22,                 //33 - ���
    Weather_Icon_Index_Hurricane = 23,                //34 - 쫷�
    Weather_Icon_Index_Tropical_Storm = 23,           //35 - �ȴ��籩
    Weather_Icon_Index_Tornado = 24,                  //36 - �����
    Weather_Icon_Index_Cold = 25,                     //37 - ��
    Weather_Icon_Index_Hot = 0,                       //38 - ��
    Weather_Icon_Index_Unknown = 255,                 //99 - δ֪
}Weather_Icon_Index_e;

//��֪���� - ��������
typedef enum _Weather_Code
{
    Weather_Code_Sunny,                     //0 - �磨���ڳ��а����磩
    Weather_Code_Clear,                     //1 - �磨���ڳ���ҹ���磩
    Weather_Code_Fair,                      //2 - �磨������а����磩
    Weather_Code_Fair_Night,                //3 - �磨�������ҹ���磩
    Weather_Code_Cloudy,                    //4 - ����
    Weather_Code_Partly_Cloudy,             //5 - ������ - ����
    Weather_Code_Partly_Cloudy_Night,       //6 - ������ - ����
    Weather_Code_Mostly_Cloudy,             //7 - �󲿶��� - ����
    Weather_Code_Mostly_Cloudy_Night,       //8 - �󲿶��� - ����
    Weather_Code_Overcast,                  //9 - ��
    Weather_Code_Shower,                    //10 - ����
    Weather_Code_Thundershower,             //11 - ������
    Weather_Code_Thundershower_with_Hail,   //12 - ��������б���
    Weather_Code_Light_Rain,                //13 - С��
    Weather_Code_Moderate_Rain,             //14 - ����
    Weather_Code_Heavy_Rain,                //15 - ����
    Weather_Code_Storm,                     //16 - ����
    Weather_Code_Heavy_Storm,               //17 - ����
    Weather_Code_Severe_Storm,              //18 - �ش���
    Weather_Code_Ice_Rain,                  //19 - ����
    Weather_Code_Sleet,                     //20 - ���ѩ
    Weather_Code_Snow_Flurry,               //21 - ��ѩ
    Weather_Code_Light_Snow,                //22 - Сѩ
    Weather_Code_Moderate_Snow,             //23 - ��ѩ
    Weather_Code_Heavy_Snow,                //24 - ��ѩ
    Weather_Code_Snowstorm,                 //25 - ��ѩ
    Weather_Code_Dust,                      //26 - ����
    Weather_Code_Sand,                      //27 - ��ɳ
    Weather_Code_Duststorm,                 //28 - ɳ����
    Weather_Code_Sandstorm,                 //29 - ǿɳ����
    Weather_Code_Foggy,                     //30 - ��
    Weather_Code_Haze,                      //31 - ��
    Weather_Code_Windy,                     //32 - ��
    Weather_Code_Blustery,                  //33 - ���
    Weather_Code_Hurricane ,                //34 - 쫷�
    Weather_Code_Tropical_Storm ,           //35 - �ȴ��籩
    Weather_Code_Tornado,                   //36 - �����
    Weather_Code_Cold,                      //37 - ��
    Weather_Code_Hot,                       //38 - ��
    Weather_Code_Unknown = 99,              //99 - δ֪
}Weather_Code_e;

void hal_lcd_test(void);

Lcd_Diplay_State_e hal_lcd_draw_font(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_str);
Lcd_Diplay_State_e hal_lcd_draw_line(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t  height, Lcd_Color_e line_color);
Lcd_Diplay_State_e hal_lcd_fill_picture(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Lcd_Color_e fill_color, Lcd_Color_e back_color, uint8_t *p_buf);


Lcd_Diplay_State_e hal_lcd_show_weather_icon(uint16_t x, uint16_t y, uint16_t weather_code);
void hal_lcd_show_temperature(uint16_t x, uint16_t y, int temperature);
void hal_lcd_show_Small_temperature(uint16_t x, uint16_t y, int temperature);
void hal_lcd_show_Date(uint16_t x, uint16_t y, uint16_t year, uint16_t month, uint16_t day, Week_e week);
void hal_lcd_show_time(uint16_t x, uint16_t y, uint8_t hour, uint8_t min, uint8_t sec);
void hal_lcd_show_small_time(uint16_t x, uint16_t y, uint8_t hour, uint8_t min);


