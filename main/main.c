#include <stdio.h>
#include "st7735.h"
#include "led.h"
#include "key.h"
#include "wifi.h"
#include "http_weather.h"
#include "hal_lcd.h"


void app_main(void)
{
    lcd_init();
    //led_init();
    //key_init();
    //wifi_init();
    hal_lcd_test();

    while (1)
    {
        //lcd_test();
        //led_test();
        
        //http_weater_test();
        vTaskDelay(1);
    }
}

