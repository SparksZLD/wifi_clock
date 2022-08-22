#include "http_weather.h"

/**
 *  1. �˴��빦�ܣ���֪��������ȡ������Ϣ
 *  2. ��ȡ����
 *     2.1 ��ȡ������Ϣ��http://ip-api.com/json/
 *     2.2 �����ȡ�������ݳɹ�����ȡ������Ϣ https://api.seniverse.com/v3/weather/now.json?key=Sb6FCTaiQBJGHuv98&location=beijing&language=zh-Hans&unit=c
 *  3. ע������: ���ڻ�ȡ������Ϣ �� �����õ��� http_get_json ��������������߳��е��ô˺�������ӻ������������ٽ���Դ
 *  4. ������Ϣ��E (2612) esp-tls-mbedtls: No server verification option set in esp_tls_cfg_t structure. Check esp_tls API reference, ��֤����֤����
 *     ���������menuconfig����-component config-ESP-TLS ��ѡ
 *          Allow potentially insecure options
 *          Skip server certificate verification by default (WARNING: ONLY FOR TESTING PURPOSE, READ HELP)
 */

static const char *TAG = "http_weather";
static char http_response_body[MAX_HTTP_OUTPUT_BUFFER] = {0};

//json ���ݽ���
static cjson_region_info_t cjson_region_info;
static cjson_weather_info_t cjson_weather_info;
static cjson_date_info_t cjson_date_info;

//����
static Date_t now_date;

/**
 * @brief ��������json����
 * @param *message, json����
 * @param *output, ������json����ͨ��ָ��ķ�ʽ��ֵ, ����������鿴 cjson_region_info_t
 * @return �������ʧ�ܷ���-1, ���򷵻�0
 */
static int city_cjson_to_struct_info(char *message, cjson_region_info_t *output)
{
#if 0
/*
    http://ip-api.com/json/ -- �������쳣
    {
        "status":"success",
        "country":"China",
        "countryCode":"CN",
        "region":"GD",
        "regionName":"Guangdong",
        "city":"Shenzhen",
        "zip":"",
        "lat":22.5579,
        "lon":114.065,
        "timezone":"Asia/Shanghai",
        "isp":"ShenZhen Topway Video Communication Co. Ltd",
        "org":"Topway",
        "as":"AS17962 ShenZhen Topway Video Communication Co. Ltd",
        "query":"115.45.57.209"
    }
*/
    /* ��������json���� */
    output->cjson_root = cJSON_Parse(message);

    if(output->cjson_root == NULL)
    {
        printf("city json parse fail.\n");
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
        return -1;
    }

    printf("city json parse succeed.\n");

    /* �������� */
    output->cjson_status = cJSON_GetObjectItem(output->cjson_root, "status");
    output->cjson_country = cJSON_GetObjectItem(output->cjson_root, "country");
    output->cjson_countryCode =  cJSON_GetObjectItem(output->cjson_root, "countryCode");
    output->cjson_region = cJSON_GetObjectItem(output->cjson_root, "region");
    output->cjson_regionName = cJSON_GetObjectItem(output->cjson_root, "regionName");
    output->cjson_city = cJSON_GetObjectItem(output->cjson_root, "city");
    output->cjson_zip = cJSON_GetObjectItem(output->cjson_root, "zip");
    output->cjson_lat = cJSON_GetObjectItem(output->cjson_root, "lat");
    output->cjson_lon = cJSON_GetObjectItem(output->cjson_root, "lon");
    output->cjson_timezone = cJSON_GetObjectItem(output->cjson_root, "timezone");
    output->cjson_isp = cJSON_GetObjectItem(output->cjson_root, "isp");
    output->cjson_org = cJSON_GetObjectItem(output->cjson_root, "org");
    output->cjson_as = cJSON_GetObjectItem(output->cjson_root, "as");
    output->cjson_query = cJSON_GetObjectItem(output->cjson_root, "query");

    printf("city:%s\n", output->cjson_city->valuestring);

    return 0;
#endif


/*
    https://ip.useragentinfo.com/json
    {
        "country": "�й�", 
        "short_name": "CN", 
        "province": "�㶫ʡ", 
        "city": "������", 
        "area": "������", 
        "isp": "������Ѷ", 
        "net": "", 
        "ip": "115.45.57.209",
        "code": 200, 
        "desc": "success"
    }
*/

    /* ��������json���� */
    output->cjson_root = cJSON_Parse(message);

    if(output->cjson_root == NULL)
    {
        printf("city json parse fail.\n");
        return -1;
    }

    /* �������� */
    output->cjson_country = cJSON_GetObjectItem(output->cjson_root, "country");
    output->cjson_short_name = cJSON_GetObjectItem(output->cjson_root, "short_name");
    output->cjson_province = cJSON_GetObjectItem(output->cjson_root, "province");
    output->cjson_city = cJSON_GetObjectItem(output->cjson_root, "city");
    output->cjson_area = cJSON_GetObjectItem(output->cjson_root, "area");
    output->cjson_isp = cJSON_GetObjectItem(output->cjson_root, "isp");
    output->cjson_net = cJSON_GetObjectItem(output->cjson_root, "net");
    output->cjson_ip = cJSON_GetObjectItem(output->cjson_root, "ip");
    output->cjson_code = cJSON_GetObjectItem(output->cjson_root, "code");
    output->cjson_desc = cJSON_GetObjectItem(output->cjson_root, "desc");

    printf("city:%s\n", output->cjson_city->valuestring);

    return 0;
}


/**
 * @brief ��������json����
 * @param *message, json����
 * @param *output, ������json����ͨ��ָ��ķ�ʽ��ֵ, ����������鿴 cjson_weather_info_t
 * @return �������ʧ�ܷ���-1, ���򷵻�0
 */
static int weather_cjson_to_struct_info(char *message, cjson_weather_info_t *output)
{
/*
    {
        "results":
        [
            {
                "location":
                {
                    "id":"WS10730EM8EV",
                    "name":"����",
                    "country":"CN",
                    "path":"����,����,�㶫,�й�",
                    "timezone":"Asia/Shanghai",
                    "timezone_offset":"+08:00"
                },

                "now":
                {
                    "text":"��",
                    "code":"9",
                    "temperature":"27"
                },

                "last_update":"2022-07-03T23:12:40+08:00"
            }
        ]
    }
*/

    cJSON *object;

    /* ��������json���� */
    output->root = cJSON_Parse(message);

    if(output->root == NULL)
    {
        printf("city json parse fail.\n");
        return -1;
    }

    printf("weather json parse succeed.\n");

    /* ��ȡ�б����� */
    output->cjson_arrayItem = cJSON_GetObjectItem(output->root, "results");

    /* ����results�ڲ����б����� */
    if(output->cjson_arrayItem != NULL)
    {
        /* ��ȡ�б��� */
        int size = cJSON_GetArraySize(output->cjson_arrayItem);
        printf("cJSON_GetArraySize: size=%d\n", size);

        for (int i = 0; i < size; i++)
        {
            printf("i=%d\n", i);
            object = cJSON_GetArrayItem(output->cjson_arrayItem, i);

            //���б��л�ȡ location ����
            output->cjson_location = cJSON_GetObjectItem(object, "location");
            if(output->cjson_location == NULL)
            {
                printf("city json parse [location] fail.\n");
                return -1;
            }
            //���� location ����
            output->cjson_location_id = cJSON_GetObjectItem(output->cjson_location, "id");
            output->cjson_location_name = cJSON_GetObjectItem(output->cjson_location, "name");
            output->cjson_location_country = cJSON_GetObjectItem(output->cjson_location, "country");
            output->cjson_location_path = cJSON_GetObjectItem(output->cjson_location, "path");
            output->cjson_location_timezone = cJSON_GetObjectItem(output->cjson_location, "timezone");
            output->cjson_location_timezone_offset = cJSON_GetObjectItem(output->cjson_location, "timezone_offset");

            //���б��л�ȡ now ����
            output->cjson_now = cJSON_GetObjectItem(object, "now");
            if(output->cjson_now == NULL)
            {
                printf("city json parse [now] fail.\n");
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                return -1;
            }
            //���� now ����
            output->cjson_now_text = cJSON_GetObjectItem(output->cjson_now, "text");
            output->cjson_now_code = cJSON_GetObjectItem(output->cjson_now, "code");
            output->cjson_now_temperature = cJSON_GetObjectItem(output->cjson_now, "temperature");

            //���б��л�ȡ last_update ����
            output->cjson_last_update = cJSON_GetObjectItem(object, "last_update");
            if(output->cjson_last_update == NULL)
            {
                printf("city json parse [last_update] fail.\n");
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                return -1;
            }
            //���� last_update ����
            output->cjson_last_update = cJSON_GetObjectItem(output->cjson_last_update, "last_updata");


            printf("weather:%s\n", output->cjson_now_text->valuestring);
        }
    }

    return 0;
}


/**
 * @brief ��������json����
 * @param *message, json����
 * @param *output, ������json����ͨ��ָ��ķ�ʽ��ֵ, ����������鿴 cjson_weather_info_t
 * @return �������ʧ�ܷ���-1, ���򷵻�0
 */
static int date_cjson_to_struct_info(char *message, cjson_date_info_t *output)
{
/*
    {
        "sysTime2":"2022-08-12 22:46:20",
        "sysTime1":"20220812224620"
    }
*/
    /* ��������json���� */
    output->root = cJSON_Parse(message);

    if(output->root == NULL)
    {
        printf("date json parse fail.\n");
        return -1;
    }

    printf("date json parse succeed.\n");

    /* �������� */
    output->sysTime1 = cJSON_GetObjectItem(output->root, "sysTime1");
    output->sysTime2 = cJSON_GetObjectItem(output->root, "sysTime2");

    return 0;
}

/**
 * @brief �����ַ���ʱ��
 * @param str_data �������������ַ���
 * @param now_date �������������
 * @return �ɹ�����0, ʧ�ܷ���-1
 * @note ������������ַ��� "20220812095735", �˺����ж���ݷ�Χ(1970��-2100��)
 */
int http_analysis_str_date(char *str_data, Date_t *now_date)
{
    int ret = 0;
    uint16_t year=0, month=0, day=0, hour=0, min=0, sec=0;
    char *p_tmp;
    
    p_tmp = str_data;

    //�����ַ���, �����ж������ַ��Ƿ�Ϲ�
    uint8_t str_example[] = "20220812095735";
    uint8_t str_example_len, str_len = 0;

    //�����ַ�������, ����ʹ�� strlen(str_example) �Ͳ��ü�1
    str_example_len = sizeof(str_example) - 1; 

    //�жϳ����Ƿ���ȷ
    while (*p_tmp)
    {
        p_tmp++;
        str_len++;
        
        if(str_len > str_example_len)
        {
            return -1;
        }
    }

    if(str_len < str_example_len)
    {
        return -1;
    }
    
    //��������
    for (int i = 0; i < str_example_len; i++)
    {
        printf("%c:%d\n", str_data[i], str_data[i]);
        //0-3�ֽ� - ���
        if(i<4)
        {
            year += pow(10, (3-i)) * (str_data[i] - '0');
        }
        //4-5�ֽ� - ��
        else if(i<6)
        {
            month +=  pow(10, (5-i)) * (str_data[i] - '0');
        }
        //6-7 - ��
        else if(i<8)
        {
            day +=  pow(10, (7-i)) * (str_data[i] - '0');
        }
        //8-9�ֽ� - ʱ
        else if(i<10)
        {
            hour +=  pow(10, (9-i)) * (str_data[i] - '0');
        }
        //10-11�ֽ� - ��
        else if(i<12)
        {
            min +=  pow(10, (11-i)) * (str_data[i] - '0');
        }
        //12-13�ֽ� - ��
        else if(i<14)
        {
            sec +=  pow(10, (13-i)) * (str_data[i] - '0');
        }
        else
        {
            return -1;
        }
    }

    //��ݺ��·��ж�
    if(year < 1970 || year > 2100 ||month == 0 || month > 12 || hour >= 24 || min >= 60 || sec >= 60)
    {
        return -1;
    }

    //day�ж�
    switch (month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(day>31)
                return -1;
            break;
        
        case 4:
        case 6:
        case 9:
        case 11:
            if(day>30)
                return -1;
            break;

        case 2:
            //�����ж�, �ܱ�4����ȴ���ܱ�100���� �� �ܱ�400��������ݾ�������
            if((year%4 == 0 && year % 100 != 0) || year%400 == 0)
            {
                //����, 2��29��
                if(day>29)
                    return -1;
            }
            else
            {
                if(day>28)
                    return -1;
            }
            break;

        default:
            break;
    }

    //���ݽ����ɹ�
    now_date->year = year;
    now_date->month = month;
    now_date->day = day;

    now_date->hour = hour;
    now_date->minute = min;
    now_date->second = sec;

    return ret;
}


/**
 * @brief ͨ��http�����ȡJSON����, JSON������󳤶Ȳο��� MAX_HTTP_OUTPUT_BUFFER��
 *        ����http_response_body��ȫ�ֱ�������������߳��е��ô˺�������ӻ������������ٽ���Դ
 * @param *url, http��������
 * @return �����ȡjson����ʧ�ܷ���-1, ���򷵻�0
 */
static int http_get_json(char *url)
{
    //char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   //���ڽ���ͨ��httpЭ�鷵�ص�����
    int content_length = 0;  //httpЭ��ͷ�ĳ���

    //�߳��е��ô˺������ڴ˴���ӻ�����
    memset(http_response_body, 0, sizeof(http_response_body));

    //02-2 ����http�ṹ��
    //����http���ýṹ�壬���ҽ�������
    esp_http_client_config_t config ;
    memset(&config,0,sizeof(config));

    //�����ýṹ���ڲ�д��url
    config.url = url;

    //��ʼ���ṹ��
    esp_http_client_handle_t client = esp_http_client_init(&config);	//��ʼ��http����

    //���÷�������
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    //��Ŀ�������������ӣ���������д�����ݳ���Ϊ0
    //��Ϊ�����post���󣬻��ڱ��ĵ�ͷ���������Ҫ����������͵�����
    //������get���������͵����ݶ���URL���棬���ڱ���ͷ��������Ҫ�������Ĳ��֣����д�볤�Ⱦ���0
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK)
    {
        //����ʧ��
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return -1;
    }
    else
    {
        //���ӳɹ�
        //��ȡĿ�������ķ������ݵ�Э��ͷ
        content_length = esp_http_client_fetch_headers(client);

        //���Э��ͷ����С��0��˵��û�гɹ���ȡ��
        if (content_length < 0)
        {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        }
        else
        {
            //��ȡĿ������ͨ��http����Ӧ����
            int data_read = esp_http_client_read_response(client, http_response_body, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
                //��ӡ��Ӧ���ݣ�������Ӧ״̬����Ӧ�峤�ȼ�������
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),				//��ȡ��Ӧ״̬��Ϣ
                esp_http_client_get_content_length(client));			//��ȡ��Ӧ��Ϣ����
                printf("data:%s\n", http_response_body);
            }
            else
            {
                //��ȡ��Ӧ��ʧ��
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }

    //�ر�����
    esp_http_client_close(client);
    return 0;
}


/**
 * @brief http��ȡ����JSON���ݣ�����������
 * @param none
 * @return none
 */
void http_get_city_info(void)
{
    int response_state = http_get_json( (char *)REGION_HRTTPS_LINK );
    if(response_state != -1)
        city_cjson_to_struct_info(http_response_body, &cjson_region_info);
}


/**
 * @brief http��ȡ����JSON���ݣ�������
 * @param none
 * @return �����ȡ��������ʧ�ܷ���-1, ���򷵻�0
 */
int http_get_weather_info(void)
{
    /*
        1.ƴ���ַ���
        "https://api.seniverse.com/v3/weather/now.json?key=" + ZHIXIN_WEATHER_KEY + "&location=" + cjson_city->valuestring + "&language=zh-Hans&unit=c"
        
        ƴ�Ӻ�"https://api.seniverse.com/v3/weather/now.json?key=Sb6FCTaiQBJGHuv98&location=beijing&language=zh-Hans&unit=c"
    */

    int http_response_state = 0, json_analysis_ret = 0, result = 0;

    char url[200] = "https://api.seniverse.com/v3/weather/now.json?key=";

    http_response_state = http_get_json( (char *)REGION_HRTTPS_LINK );
    if(http_response_state != 0)
    {
        result = -1;
    }
    else
    {
        /* ��������json���� */
        json_analysis_ret = city_cjson_to_struct_info(http_response_body, &cjson_region_info);

        if(json_analysis_ret != -1)
        {
            /* ����������ݽ����ɹ�����ʼƴ���ַ��� */
            strcat(url, XINZHI_WEATHER_KEY);
            strcat(url, "&location=");
            strcat(url, cjson_region_info.cjson_city->valuestring);
            strcat(url, "&language=zh-Hans&unit=c");
            printf("url = %s\n", url);

            http_response_state  = http_get_json( url );
            if(http_response_state != -1)
            {
                json_analysis_ret = weather_cjson_to_struct_info(http_response_body, &cjson_weather_info);
                if(json_analysis_ret != -1)
                {
                    //�������ݽ����ɹ�
                }
                else
                {
                    result = -1;
                }
            }
        }
    }

    if(result == -1)
    {
        printf("��ȡ����json����ʧ��\n");
    }

    return result;
}



/**
 * @brief http��ȡ����JSON���ݣ�����������
 * @param none
 * @return �����ȡ��������ʧ�ܷ���-1, ���򷵻�0
 */
int http_get_date_info(void)
{
    int ret = 0;
    int response_state = http_get_json( (char *)BEIJING_TIME_LINK );
    if(response_state == -1)
    {
        return -1;
    }

    //��ȡ���ݳɹ�
    ret = date_cjson_to_struct_info(http_response_body, &cjson_date_info);
    if(ret == -1)
        return -1;
    
    //���ַ�������ת��Ϊ�������� - ������ʱ����
    ret = http_analysis_str_date(cjson_date_info.sysTime2->valuestring, &now_date);
    if(ret == -1)
        return -1;

    //��ȡ���� 
    now_date.week = get_Week(now_date.year, now_date.month, now_date.day);

    return 0;
}

void http_weater_test(void)
{
    http_get_weather_info();
}
