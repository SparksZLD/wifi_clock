#pragma once

/*********************************** Header file include ******************************************/
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"


#define LCD_WIDTH         240
#define LCD_HEIGHT        240


/************************************ Function definition *******************************************/
/**
 * @brief 将RGB(888) 转换为 RGB(565)
 * @param color - 颜色值，低24位有效
 * @return 返回转换后的颜色值
 */
#define Color_888_To_565(color) 
/********************************** Function definition end *****************************************/


/********************************* Parameter type definition ******************************************/
typedef enum 
{
    COLOR_BLACK = 0x0000,
    COLOR_WHITE = 0xFFFF,
    COLOR_RED   = 0xF800,
    COLOR_GREEN = 0x07E0,
    COLOR_BLUE  = 0x001F,
}Lcd_Color_e;

/******************************* Parameter type definition end ****************************************/


void lcd_init(void);
void lcd_test(void);
void lcd_clear(uint16_t color);

void st7735_data(const uint8_t *data, int len);
void lcd_address_window_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
//void lcd_backlight_set(uint8_t x);

