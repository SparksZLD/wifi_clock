#pragma once

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "wifi.h"


/* 按键IO口 */
#define PIN_NUM_KEY               0

/* 按键按下时电平 */
#define KEY_PRESS_LEVEL           0 

/* 短按，计数范围区间 */
#define KEY_SHORT_LOWER_LIMIT     5
#define KEY_SHORT_UPPER_LIMIT     20

/* 长按，计数范围区间 */
#define KEY_LONG_LOWER_LIMIT      500

typedef enum{
    LED_OFF = 0,
    LED_ON,
}LED_STATE_t;

void led_init(void);
void led_set(LED_STATE_t x);
void led_test(void);

