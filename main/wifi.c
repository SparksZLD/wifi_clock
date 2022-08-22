/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "led.h"


/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smartconfig_example";

static TaskHandle_t *pvSmartconfigTask = NULL; //����������

static void smartconfig_example_task(void * parm);

/**
 * @brief ��ȡwifi������Ϣ, ���� wifi ����Ϊվ��ģʽ
 * @param none
 * @return none
 */
static void wifi_station_config(void)
{
    wifi_config_t wifi_config;
    
    //����
    bzero(&wifi_config, sizeof(wifi_config));

    //��ȡ nvs �д�ȡ��wifi��Ϣ
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
}

/**
 * @brief 
 * 
 */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        /* ����wifi */
        wifi_station_config();
        esp_wifi_connect();
        ESP_LOGI(TAG, "wifi start");
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        /* �¼����ͣ�WIFI_EVENT, wifi�Ͽ� */
        /* wifi�Ͽ����� */
        ESP_LOGI(TAG, "wifi disconnect, try to reconnect");
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        /* �¼����ͣ�IP_EVENT, ��ȡIP */
        /* �����ӵ�Ap�л�IP, ���� wifi connect�¼� */
        ESP_LOGI(TAG, "wifi got ip");
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {
        /* �¼����ͣ�SC_EVENT, wifiɨ�����, ��ȡAP */
        ESP_LOGI(TAG, "Scan done");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        /* �¼����ͣ�SC_EVENT, ����Ŀ��APͨ�� */
        ESP_LOGI(TAG, "Found channel");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        /* �¼����ͣ�SC_EVENT, ��ȡAP ssid �� passwd */
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        /* ��ʼ��(��0) wifi_config ���� */
        bzero(&wifi_config, sizeof(wifi_config_t));

        /* evt->ssid ���Ƶ� wifi_config.sta.ssid */
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        /* evt->password ���Ƶ� wifi_config.sta.password */
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        /* evt->bssid_set(�Ƿ�����Ŀ��AP��MAC��ַ bool ����) ��ֵ��  wifi_config.sta.bssid_set */
        wifi_config.sta.bssid_set = evt->bssid_set;

        if (wifi_config.sta.bssid_set == true) 
        {
            /* ����Ŀ��AP��MAC��ַ, evt->bssid ���Ƶ� wifi_config.sta.bssid */
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        /* �� evt->ssid ���Ƶ� ssid ������ */
        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        /* �� evt->ssid ���Ƶ� password ������ */
        memcpy(password, evt->password, sizeof(evt->password));

        /* ��ӡ ssid �� password */
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);

        if (evt->type == SC_TYPE_ESPTOUCH_V2) 
        {
            /* ��ȡESPTouch v2��Ԥ������, �����  */
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");

            for (int i=0; i<33; i++) 
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        /* �Ͽ�wifi���� */
        ESP_ERROR_CHECK( esp_wifi_disconnect() );

        /* ����wifi���� */
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

        /* ����wifi */
        esp_wifi_connect();
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        /* ESP32վsmartconfig�����ֻ�����ACK, ESPTouch Done */
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

/**
 * @brief wifi վ��ģʽ��ʼ��
 * @param none
 * @return none
 */
static void initialise_wifi(void)
{
    //station config
    /* ��ʼ���ײ�TCP/IP��ջ */
    ESP_ERROR_CHECK(esp_netif_init());

    /* ����һ���¼��� smartconfig �¼��� */
    s_wifi_event_group = xEventGroupCreate();

    /* wifi�¼� */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    /* wifi��ʼ�� */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    /* ע���¼� */
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) ); //���¼����ͣ�WIFI_EVENT ��  ESP_EVENT_ANY_ID(ȫ���¼�)   ע�ᵽ event_handler ����
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );//���¼����ͣ�IP_EVENT �� IP_EVENT_STA_GOT_IP(��ȡIP�¼�) ע�ᵽ event_handler ����
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );   //���¼����ͣ�SC_EVENT �� ESP_EVENT_ANY_ID(ȫ���¼�) ע�ᵽ event_handler ����

    #if 0
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "TP-LINK_0D7D",
            .password = "z123456789",
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    #else
    //wifi_station_config();
    #endif

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    //ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}


/**
 * @brief smart config ����
 * @param parm - �̶�д��, void *parm, �������ָ��
 * @return none
 */
static void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;

    /* ���� smart config Э������ */
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );

    while (1) 
    {
        /* �ȴ��¼� */
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) 
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");

            //��� wifi ���ӳɹ�������
            led_set(LED_OFF);
        }

        if(uxBits & ESPTOUCH_DONE_BIT) 
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();

            /* ��������Ϣ������nvs flash�� */
            ESP_LOGI(TAG, "storage wifi congfig to nvs flash");
            esp_wifi_set_storage(WIFI_STORAGE_FLASH);   

            /* ɾ�����߳� */
            vTaskDelete(NULL);

            pvSmartconfigTask = NULL;
        }
    }
}

/**
 * @brief wifi �豸��ʼ��
 * @param none
 * @return none
 */
void wifi_init(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
}


/**
 * @brief ���� smart config 
 * @param none
 * @return none
 */
void wifi_smartconfig_start(void)
{
    // xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    if(pvSmartconfigTask == NULL)
    {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, pvSmartconfigTask);
        ESP_LOGI(TAG, "start smartconfig");
        led_set(LED_ON);
    }
}


/**
 * @brief �ر� smart config
 * @param none
 * @param none
 */
void wifi_smartconfig_stop(void)
{
    if(pvSmartconfigTask != NULL)
    {
        //ɾ���߳�
        vTaskDelete(pvSmartconfigTask);

        esp_smartconfig_stop();

        //��������wifi
        wifi_station_config();
        esp_wifi_connect();

        led_set(LED_OFF);
        pvSmartconfigTask = NULL;
    }
}



#if 0
//���ڲ��� WiFi �Ƿ�洢��nvs��
void wifi_test(void)
{
    wifi_config_t wifi_config;
    /* ��ʼ��(��0) wifi_config ���� */
    bzero(&wifi_config, sizeof(wifi_config_t));

    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);


    printf("read wifi ssid from nvs: %s\n", wifi_config.sta.ssid);
    printf("read wifi passwd from nvs: %s\n", wifi_config.sta.password);

    while(1)
    {
        vTaskDelay(1000);
    }
}
#endif


