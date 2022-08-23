#include <stdio.h>
#include "st7735.h"
#include "led.h"
#include "key.h"
#include "wifi.h"
#include "http_weather.h"
#include "hal_lcd.h"


/*事件的句柄*/
static EventGroupHandle_t App_Init_Event_Handler = NULL;
#define LCD_INIT_FINISH_EVENT       (0x01 << 0)
#define WIFI_INIT_FINISH_EVENT      (0x01 << 1)


void app_init(void);

void app_main(void)
{
    
    app_init();

    hal_lcd_test();

    while (1)
    {
        //lcd_test();
        //led_test();
        
        //http_weater_test();
        vTaskDelay(1);
    }
}


/**
 * @brief lcd初始化
 * @param *pvParameters, 任意类型参数, freeos任务回调函数固定写法
 * @return none
 */
void app_lcd_init(void *pvParameters)
{
    lcd_init();

    //发送lcd初始化完成事件
    xEventGroupSetBits(App_Init_Event_Handler, LCD_INIT_FINISH_EVENT);

    //销毁任务
    vTaskDelete(NULL);
}

/**
 * @brief wifi 初始化
 * @param *pvParameters, 任意类型参数, freeos任务回调函数固定写法
 * @return none
 */
void app_wifi_init(void *pvParameters)
{
    EventBits_t r_event = pdPASS;
    while (1)
    {
        r_event = xEventGroupWaitBits(App_Init_Event_Handler,      //事件的句柄
                                      LCD_INIT_FINISH_EVENT,       //感兴趣的事件
                                      pdTRUE,                      //退出时是否清除事件位
                                      pdTRUE,                      //是否满足所有事件
                                      portMAX_DELAY);              //超时时间，一直等所有事件都满足

        if (r_event == LCD_INIT_FINISH_EVENT)
        {
            wifi_init();

            //发送wifi初始化完成事件
            xEventGroupSetBits(App_Init_Event_Handler, WIFI_INIT_FINISH_EVENT);

            //销毁任务
            vTaskDelete(NULL);
        }
    }
}

/**
 * @brief 获取网络数据
 * @param *pvParameters, 任意类型参数, freeos任务回调函数固定写法
 * @return none
 */
void app_get_http_data(void *pvParameters)
{
    EventBits_t r_event = pdPASS;
    wifi_ap_record_t ap_info;
    esp_err_t ret;
    uint8_t flag=0;

    while (1)
    {
        r_event = xEventGroupWaitBits(App_Init_Event_Handler,      //事件的句柄
                                      WIFI_INIT_FINISH_EVENT,       //感兴趣的事件
                                      pdTRUE,                      //退出时是否清除事件位
                                      pdTRUE,                      //是否满足所有事件
                                      portMAX_DELAY);              //超时时间，一直等所有事件都满足

        if(r_event != WIFI_INIT_FINISH_EVENT)
        {
            vTaskDelete(NULL);
            return;
        }

        for (int i = 0; i < 5; i++)
        {
            //等待5秒, 查看有没有连接wifi
            ret = esp_wifi_sta_get_ap_info(&ap_info);

            //if(ret == ESP_ERR_WIFI_NOT_CONNECT)  // wifi没有连接
            if(ESP_OK == ret)
            {
                //wifi 已经连接
                flag = 1;
                break;
            }

            vTaskDelay( 1000/ portTICK_PERIOD_MS );
        }

        if(flag)
        {
            //获取网络数据
            if( (http_get_date_info() ==  -1) ||  (http_get_weather_info() == -1) )
            {
                vTaskDelete(NULL);
                return;
            }
        }

        //销毁任务
        vTaskDelete(NULL);
    }
}


/**
 * @brief 设备初始化
 * @param none
 * @return none
 */
void app_init(void)
{   
    //1.按键初始化
    key_init();

    //2.LED初始化
    led_init();

    //创建事件
    App_Init_Event_Handler = xEventGroupCreate();

    if (App_Init_Event_Handler != NULL)
        printf("创建初始化事件成功\n");

    //3.创建任务初始化 LCD 
    xTaskCreate(app_lcd_init, "lcd_init", 2048, NULL, 3, NULL);

    //4.创建任务等待 LCD 初始化完成，然后初始化 wifi
    xTaskCreate(app_wifi_init, "wifi_init", 2048, NULL, 3, NULL);

    //创建任务等待 wifi 初始化完成, 并等待wifi连接, wifi连接后请求网络数据
    xTaskCreate(app_get_http_data, "get_http_data", 2048, NULL, 3, NULL);
}


