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

//日期显示位置
#define LCD_DATE_SHOW_X                         0
#define LCD_DATE_SHOW_Y                         55
//日期显示颜色
#define LCD_DATE_SHOW_COLOR                     COLOR_WHITE
#define LCD_DATE_SHOW_BACK_COLOR                COLOR_BLACK

//时间(大字) - 显示位置
#define LCD_TIME_SHOW_X                         56
#define LCD_TIME_SHOW_Y                         103
//时间(大字) - 显示颜色
#define LCD_TIME_SHOW_COLOR                     COLOR_WHITE
#define LCD_TIME_SHOW_BACK_COLOR                COLOR_BLACK

//横线位置
#define LCD_LINE_X                              0
#define LCD_LINE_Y                              150
//线粗
#define LCD_LINE_HEIGHT                         1
//横线颜色
#define LCD_LINE_COLOR                          COLOR_WHITE

//时间(小字) - 显示位置
#define  LCD_SMALL_TIME_SHOW_X                  0
#define  LCD_SMALL_TIME_SHOW_Y                  155
//时间(小字) - 显示颜色
#define  LCD_SMALL_TIME_SHOW_COLOR              COLOR_WHITE
#define  LCD_SMALL_TIME_SHOW_BACK_COLOR         COLOR_BLACK

//温度(小字) - 显示位置
#define  LCD_TEMP_SMALL_SHOW_X                  199
#define  LCD_TEMP_SMALL_SHOW_Y                  155
//温度(小字) - 显示颜色
#define  LCD_TEMP_SMALL_SHOW_COLOR              COLOR_WHITE
#define  LCD_TEMP_SMALL_SHOW_BACK_COLOR         COLOR_BLACK

//温度(大字) - 显示位置
#define  LCD_TEMP_SHOW_X                        120
#define  LCD_TEMP_SHOW_Y                        82
//温度(大字) - 显示颜色
#define  LCD_TEMP_SHOW_COLOR                    COLOR_WHITE
#define  LCD_TEMP_SHOW_BACK_COLOR               COLOR_BLACK

//天气图标刷新位置
#define LCD_WEATHER_ICON_X                      56
#define LCD_WEATHER_ICON_Y                      70
//天气图标大小
#define LCD_WEATHER_ICON_WIDTH                  56
#define LCD_WEATHER_ICON_HEIGHT                 56
//天气图标颜色
#define LCD_WEATHER_ICON_COLOR                  COLOR_WHITE
#define LCD_WEATHER_ICON_BACK_COLOR             COLOR_BLACK

//刷新屏时返回的状态
typedef enum
{
    LCD_DISPLAY_OK             = 0,      //显示正常
    LCD_DISPLAY_ADDRESS_ERR    = 1,      //显示地址溢出
    LCD_DISPLAY_NOT_FOUND_ERR  = 2,      //未找到显示的数组
    LCD_DISPLAY_OVERFLOW_ERR   = 4,      //刷新区域越界(溢出屏幕)
    LCD_DISPLAY_AREA_ERR       = 5,      //划分的区域错误, 例如划分一块宽高都为0的区域

    LCD_DISPLAY_SLIDE_STEP_ERR = 6,      //幻灯片步长错误
    LCD_DISPLAY_SLIDE_DIR_ERR  = 7,      //幻灯片方向错误
}Lcd_Diplay_State_e;

//幻灯片移动移动方向
typedef enum _lcd_slide_move_direction
{
    Lcd_Slide_Top,
    Lcd_Slide_Bottom,
    Lcd_Slide_Left,
    Lcd_Slide_Right,
}Lcd_Slide_Dir_e;

//幻灯片配置
typedef struct _lcd_slide_config
{
    uint16_t x;                //x轴
    uint16_t y;                //y轴
    uint16_t width;            //宽, 幻灯片画布宽, 8的倍数
    uint16_t height;           //高, 幻灯片画布高, 8的倍数
    
    Lcd_Slide_Dir_e slide_dir; //幻灯片移动方向
    uint16_t speed_ms;         //幻灯片速度, 多少毫秒移动一像素

    uint16_t src_color;        //当前显示的界面字体/图案颜色
    uint16_t src_back_color;   //当前显示界面的背景色
    uint8_t *src_buf;          //src_buf 大小必须是 width*height/8, 否则会乱码

    uint16_t dest_color;       //目标显示的界面字体/图案颜色
    uint16_t dest_back_color;  //目标显示界面的背景色
    uint8_t *dest_buf;         //dest_buf 大小必须是 width*height/8, 否则会乱码

    uint8_t step_len;          //步长, 一次移动多少像素, 如果移动方式是top 或 bottom, step_len < height即可, 但是如果移动方式是left 或 right, step_len必须是8的倍数

}Lcd_Slide_Config_t;


//心知天气 - 图标索引
typedef enum _Weather_Icon_Index
{
    Weather_Icon_Index_Sunny = 0,                     //0 - 晴（国内城市白天晴）
    Weather_Icon_Index_Clear = 1,                     //1 - 晴（国内城市夜晚晴）
    Weather_Icon_Index_Fair = 0,                      //2 - 晴（国外城市白天晴）
    Weather_Icon_Index_Fair_Night = 3,                //3 - 晴（国外城市夜晚晴）
    Weather_Icon_Index_Cloudy = 2,                    //4 - 多云
    Weather_Icon_Index_Partly_Cloudy = 3,             //5 - 晴间多云 - 白天
    Weather_Icon_Index_Partly_Cloudy_Night = 4,       //6 - 晴间多云 - 晚上
    Weather_Icon_Index_Mostly_Cloudy = 3,             //7 - 大部多云 - 白天
    Weather_Icon_Index_Mostly_Cloudy_Night = 4,       //8 - 大部多云 - 晚上
    Weather_Icon_Index_Overcast = 5,                  //9 - 阴
    Weather_Icon_Index_Shower = 6,                    //10 - 阵雨
    Weather_Icon_Index_Thundershower = 7,             //11 - 雷阵雨
    Weather_Icon_Index_Thundershower_with_Hail = 8,   //12 - 雷阵雨伴有冰雹
    Weather_Icon_Index_Light_Rain = 9,                //13 - 小雨
    Weather_Icon_Index_Moderate_Rain = 10,            //14 - 中雨
    Weather_Icon_Index_Heavy_Rain = 11,               //15 - 大雨
    Weather_Icon_Index_Storm = 12,                    //16 - 暴雨
    Weather_Icon_Index_Heavy_Storm = 12,              //17 - 大暴雨
    Weather_Icon_Index_Severe_Storm = 12,             //18 - 特大暴雨
    Weather_Icon_Index_Ice_Rain = 10,                 //19 - 冻雨
    Weather_Icon_Index_Sleet = 13,                    //20 - 雨夹雪
    Weather_Icon_Index_Snow_Flurry = 14,              //21 - 阵雪
    Weather_Icon_Index_Light_Snow = 15,               //22 - 小雪
    Weather_Icon_Index_Moderate_Snow = 16,            //23 - 中雪
    Weather_Icon_Index_Heavy_Snow = 17,               //24 - 大雪
    Weather_Icon_Index_Snowstorm = 17,                //25 - 暴雪
    Weather_Icon_Index_Dust = 18,                     //26 - 浮尘
    Weather_Icon_Index_Sand = 18,                     //27 - 扬沙
    Weather_Icon_Index_Duststorm = 19,                //28 - 沙尘暴
    Weather_Icon_Index_Sandstorm = 19,                //29 - 强沙尘暴
    Weather_Icon_Index_Foggy = 20,                    //30 - 雾
    Weather_Icon_Index_Haze = 21,                     //31 - 霾
    Weather_Icon_Index_Windy = 22,                    //32 - 风
    Weather_Icon_Index_Blustery = 22,                 //33 - 大风
    Weather_Icon_Index_Hurricane = 23,                //34 - 飓风
    Weather_Icon_Index_Tropical_Storm = 23,           //35 - 热带风暴
    Weather_Icon_Index_Tornado = 24,                  //36 - 龙卷风
    Weather_Icon_Index_Cold = 25,                     //37 - 冷
    Weather_Icon_Index_Hot = 0,                       //38 - 热
    Weather_Icon_Index_Unknown = 255,                 //99 - 未知
}Weather_Icon_Index_e;

//心知天气 - 天气代码
typedef enum _Weather_Code
{
    Weather_Code_Sunny,                     //0 - 晴（国内城市白天晴）
    Weather_Code_Clear,                     //1 - 晴（国内城市夜晚晴）
    Weather_Code_Fair,                      //2 - 晴（国外城市白天晴）
    Weather_Code_Fair_Night,                //3 - 晴（国外城市夜晚晴）
    Weather_Code_Cloudy,                    //4 - 多云
    Weather_Code_Partly_Cloudy,             //5 - 晴间多云 - 白天
    Weather_Code_Partly_Cloudy_Night,       //6 - 晴间多云 - 晚上
    Weather_Code_Mostly_Cloudy,             //7 - 大部多云 - 白天
    Weather_Code_Mostly_Cloudy_Night,       //8 - 大部多云 - 晚上
    Weather_Code_Overcast,                  //9 - 阴
    Weather_Code_Shower,                    //10 - 阵雨
    Weather_Code_Thundershower,             //11 - 雷阵雨
    Weather_Code_Thundershower_with_Hail,   //12 - 雷阵雨伴有冰雹
    Weather_Code_Light_Rain,                //13 - 小雨
    Weather_Code_Moderate_Rain,             //14 - 中雨
    Weather_Code_Heavy_Rain,                //15 - 大雨
    Weather_Code_Storm,                     //16 - 暴雨
    Weather_Code_Heavy_Storm,               //17 - 大暴雨
    Weather_Code_Severe_Storm,              //18 - 特大暴雨
    Weather_Code_Ice_Rain,                  //19 - 冻雨
    Weather_Code_Sleet,                     //20 - 雨夹雪
    Weather_Code_Snow_Flurry,               //21 - 阵雪
    Weather_Code_Light_Snow,                //22 - 小雪
    Weather_Code_Moderate_Snow,             //23 - 中雪
    Weather_Code_Heavy_Snow,                //24 - 大雪
    Weather_Code_Snowstorm,                 //25 - 暴雪
    Weather_Code_Dust,                      //26 - 浮尘
    Weather_Code_Sand,                      //27 - 扬沙
    Weather_Code_Duststorm,                 //28 - 沙尘暴
    Weather_Code_Sandstorm,                 //29 - 强沙尘暴
    Weather_Code_Foggy,                     //30 - 雾
    Weather_Code_Haze,                      //31 - 霾
    Weather_Code_Windy,                     //32 - 风
    Weather_Code_Blustery,                  //33 - 大风
    Weather_Code_Hurricane ,                //34 - 飓风
    Weather_Code_Tropical_Storm ,           //35 - 热带风暴
    Weather_Code_Tornado,                   //36 - 龙卷风
    Weather_Code_Cold,                      //37 - 冷
    Weather_Code_Hot,                       //38 - 热
    Weather_Code_Unknown = 99,              //99 - 未知
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


