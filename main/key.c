#include "led.h"

/**
 * 1.此代码功能，按键扫描、按键动作处理
 * 2.函数流程
 *   2.1 创建按键扫描任务，分为短按 和 长按。如果按键按下，不松开进入长按，会先触发短按，然后触发长按
 *   2.2 创建按键处理任务，一直等待按键按下事件
 */

/*事件的句柄*/
static EventGroupHandle_t Event_Handler = NULL;
static const char *TAG = "KEY_DEBUG";

/* 任务句柄 */
static TaskHandle_t xHandle_key_scan_task = NULL;     //按键扫描任务
static TaskHandle_t xHandle_key_handle_task = NULL;   //按键处理任务

#define KEY_SHOET_PRESS_EVENT (0x01 << 0)
#define KEY_LONG_PRESS_EVENT  (0x01 << 1)



/**
 * @brief 根据按下时间，发送按键事件，此函数内部有按键消抖
 * @param key_press_cnt 按键按下计数
 * @return none
 */
void key_send_event(int key_press_cnt)
{
    static uint8_t key_short_send_time = 0;
    static uint8_t key_long_send_time = 0;

    /*  按键没有按下，清除标志 */
    if(key_press_cnt == 0)
    {
        key_short_send_time = 0;
        key_long_send_time = 0;
        return;
    }

    //ESP_LOGI(TAG, "%d - %d - %d", key_press_cnt, key_long_send_time, key_long_send_time);

    /* 按键按下发送 按键短按事件 或 长按事件 */
    if(key_press_cnt >= KEY_SHORT_LOWER_LIMIT &&\
        key_press_cnt < KEY_SHORT_UPPER_LIMIT &&\
        key_short_send_time == 0)
    {
        key_short_send_time++;
        
        // 发送按键短按事件，进入临界区，需要保护临界资源
        xEventGroupSetBits(Event_Handler, KEY_SHOET_PRESS_EVENT);
        ESP_LOGI(TAG, "key short press...");
    }
    else if(key_press_cnt > KEY_LONG_LOWER_LIMIT && key_long_send_time == 0)
    {
        key_long_send_time++;

        //发送按键长按事件
        xEventGroupSetBits(Event_Handler, KEY_LONG_PRESS_EVENT);
        ESP_LOGI(TAG, "key long press...");
    }
}


/**
 * @brief 按键扫描任务, 短按：< 500ms，长按：3秒以上
 * @param *pvParameters, 任意类型参数, freeos任务回调函数固定写法
 * @return none
 */
void key_scan_task(void *pvParameters)
{
    static int key_press_cnt= 0;

    while (1)
    {
        if(gpio_get_level(PIN_NUM_KEY) == KEY_PRESS_LEVEL)
        {
            key_press_cnt++;
            key_send_event(key_press_cnt);
        }
        else
        {
            key_press_cnt = 0;
            key_send_event(key_press_cnt); //传入0，复位 key_send_event 内部静态变量
        }
        
        /* 按键扫描间隔10ms，任务延时函数，释放CPU使用权 */
        vTaskDelay(10/ portTICK_RATE_MS);
    } 
}


/**
 * @brief 按键处理函数
 * @param *pvParameters, 任意类型参数, freeos任务回调函数固定写法
 * @return none
 */
void key_event_handle(void *pvParameters)
{
    EventBits_t r_event = pdPASS;
    while (1)
    {
        r_event = xEventGroupWaitBits(Event_Handler,                         //事件的句柄
                            KEY_SHOET_PRESS_EVENT | KEY_LONG_PRESS_EVENT,    //感兴趣的事件
                            pdTRUE,                                          //退出时是否清除事件位
                            pdFALSE,                                         //是否满足所有事件
                            portMAX_DELAY);                                  //超时时间，一直等所有事件都满足

        if((KEY_SHOET_PRESS_EVENT & r_event) == KEY_SHOET_PRESS_EVENT)
        {
            //添加按键短按处理函数
            ESP_LOGI(TAG, "key short press handle...");
            wifi_smartconfig_stop();
        }
        else if((KEY_LONG_PRESS_EVENT & r_event) == KEY_LONG_PRESS_EVENT)
        {
            //添加按键长按处理函数
            ESP_LOGI(TAG, "key long press handle...");
            wifi_smartconfig_start();

            wifi_smartconfig_start();
        }
    }
}


/**
 * @brief 按键初始化. 
 *        1. 创建按键事件
 *        2. 创建按键扫描任务
 *        3. 创建按键处理任务
 * @param none
 * @return none
 */
void key_init(void)
{
    /* key io mode init */
    gpio_set_direction(PIN_NUM_KEY, GPIO_MODE_INPUT);

    /* 创建事件 */
    Event_Handler = xEventGroupCreate();

    if (Event_Handler != NULL)
        ESP_LOGI(TAG, "创建事件成功");


    //创建按键扫描任务
    xTaskCreatePinnedToCore(key_scan_task,          //任务函数
                            "key_scan",             //任务名称
                            2048,                   //堆栈大小
                            NULL,                   //传递参数
                            0,                      //任务优先级
                            &xHandle_key_scan_task, //任务句柄
                            tskNO_AFFINITY);        //无关联，不绑定在任何一个核上

    //创建按键处理线程
    xTaskCreatePinnedToCore(key_event_handle,         //任务函数
                            "key_handle",             //任务名称
                            2048,                     //堆栈大小
                            NULL,                     //传递参数
                            0,                        //任务优先级
                            &xHandle_key_handle_task, //任务句柄
                            tskNO_AFFINITY);          //无关联，不绑定在任何一个核上
}
