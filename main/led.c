#include "led.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_NUM_LED   2

static const char *TAG = "LED_DEBUG";

/**
 * @brief LED 初始化
 * @param none
 * @return none
 */
void led_init(void)
{
    gpio_reset_pin(PIN_NUM_LED);
    gpio_set_direction(PIN_NUM_LED, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_LED, 0);
    ESP_LOGI(TAG, "LED gpio init finished...");
}

/**
 * @brief LED 控制
 * @param x - 详情请查看 Led_State_e
 * @return none
 */
void led_set(Led_State_e x)
{
    if(x == LED_ON)
        gpio_set_level(PIN_NUM_LED, 1);
    else
        gpio_set_level(PIN_NUM_LED, 0);
}


void led_test(void)
{
    static uint8_t i=0;
    i = !i;
    led_set(i);
    vTaskDelay(500 / portTICK_RATE_MS);
}


