#include "led.h"

/**
 * 1.�˴��빦�ܣ�����ɨ�衢������������
 * 2.��������
 *   2.1 ��������ɨ�����񣬷�Ϊ�̰� �� ����������������£����ɿ����볤�������ȴ����̰���Ȼ�󴥷�����
 *   2.2 ����������������һֱ�ȴ����������¼�
 */

/*�¼��ľ��*/
static EventGroupHandle_t Event_Handler = NULL;
static const char *TAG = "KEY_DEBUG";

/* ������ */
static TaskHandle_t xHandle_key_scan_task = NULL;     //����ɨ������
static TaskHandle_t xHandle_key_handle_task = NULL;   //������������

#define KEY_SHOET_PRESS_EVENT (0x01 << 0)
#define KEY_LONG_PRESS_EVENT  (0x01 << 1)



/**
 * @brief ���ݰ���ʱ�䣬���Ͱ����¼����˺����ڲ��а�������
 * @param key_press_cnt �������¼���
 * @return none
 */
void key_send_event(int key_press_cnt)
{
    static uint8_t key_short_send_time = 0;
    static uint8_t key_long_send_time = 0;

    /*  ����û�а��£������־ */
    if(key_press_cnt == 0)
    {
        key_short_send_time = 0;
        key_long_send_time = 0;
        return;
    }

    //ESP_LOGI(TAG, "%d - %d - %d", key_press_cnt, key_long_send_time, key_long_send_time);

    /* �������·��� �����̰��¼� �� �����¼� */
    if(key_press_cnt >= KEY_SHORT_LOWER_LIMIT &&\
        key_press_cnt < KEY_SHORT_UPPER_LIMIT &&\
        key_short_send_time == 0)
    {
        key_short_send_time++;
        
        // ���Ͱ����̰��¼��������ٽ�������Ҫ�����ٽ���Դ
        xEventGroupSetBits(Event_Handler, KEY_SHOET_PRESS_EVENT);
        ESP_LOGI(TAG, "key short press...");
    }
    else if(key_press_cnt > KEY_LONG_LOWER_LIMIT && key_long_send_time == 0)
    {
        key_long_send_time++;

        //���Ͱ��������¼�
        xEventGroupSetBits(Event_Handler, KEY_LONG_PRESS_EVENT);
        ESP_LOGI(TAG, "key long press...");
    }
}


/**
 * @brief ����ɨ������, �̰���< 500ms��������3������
 * @param *pvParameters, �������Ͳ���, freeos����ص������̶�д��
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
            key_send_event(key_press_cnt); //����0����λ key_send_event �ڲ���̬����
        }
        
        /* ����ɨ����10ms��������ʱ�������ͷ�CPUʹ��Ȩ */
        vTaskDelay(10/ portTICK_RATE_MS);
    } 
}


/**
 * @brief ����������
 * @param *pvParameters, �������Ͳ���, freeos����ص������̶�д��
 * @return none
 */
void key_event_handle(void *pvParameters)
{
    EventBits_t r_event = pdPASS;
    while (1)
    {
        r_event = xEventGroupWaitBits(Event_Handler,                         //�¼��ľ��
                            KEY_SHOET_PRESS_EVENT | KEY_LONG_PRESS_EVENT,    //����Ȥ���¼�
                            pdTRUE,                                          //�˳�ʱ�Ƿ�����¼�λ
                            pdFALSE,                                         //�Ƿ����������¼�
                            portMAX_DELAY);                                  //��ʱʱ�䣬һֱ�������¼�������

        if((KEY_SHOET_PRESS_EVENT & r_event) == KEY_SHOET_PRESS_EVENT)
        {
            //��Ӱ����̰�������
            ESP_LOGI(TAG, "key short press handle...");
            wifi_smartconfig_stop();
        }
        else if((KEY_LONG_PRESS_EVENT & r_event) == KEY_LONG_PRESS_EVENT)
        {
            //��Ӱ�������������
            ESP_LOGI(TAG, "key long press handle...");
            wifi_smartconfig_start();

            wifi_smartconfig_start();
        }
    }
}


/**
 * @brief ������ʼ��. 
 *        1. ���������¼�
 *        2. ��������ɨ������
 *        3. ����������������
 * @param none
 * @return none
 */
void key_init(void)
{
    /* key io mode init */
    gpio_set_direction(PIN_NUM_KEY, GPIO_MODE_INPUT);

    /* �����¼� */
    Event_Handler = xEventGroupCreate();

    if (Event_Handler != NULL)
        ESP_LOGI(TAG, "�����¼��ɹ�");


    //��������ɨ������
    xTaskCreatePinnedToCore(key_scan_task,          //������
                            "key_scan",             //��������
                            2048,                   //��ջ��С
                            NULL,                   //���ݲ���
                            0,                      //�������ȼ�
                            &xHandle_key_scan_task, //������
                            tskNO_AFFINITY);        //�޹������������κ�һ������

    //�������������߳�
    xTaskCreatePinnedToCore(key_event_handle,         //������
                            "key_handle",             //��������
                            2048,                     //��ջ��С
                            NULL,                     //���ݲ���
                            0,                        //�������ȼ�
                            &xHandle_key_handle_task, //������
                            tskNO_AFFINITY);          //�޹������������κ�һ������
}
