#include "http_weather.h"

/**
 *  1. 此代码功能，从知心天气获取天气信息
 *  2. 获取流程
 *     2.1 获取城市信息，http://ip-api.com/json/
 *     2.2 如果获取城市数据成功，获取天气信息 https://api.seniverse.com/v3/weather/now.json?key=Sb6FCTaiQBJGHuv98&location=beijing&language=zh-Hans&unit=c
 *  3. 注意事项: 由于获取城市信息 和 天气用的是 http_get_json 函数。如果是在线程中调用此函，需添加互斥锁，保护临界资源
 *  4. 错误信息：E (2612) esp-tls-mbedtls: No server verification option set in esp_tls_cfg_t structure. Check esp_tls API reference, 是证书认证问题
 *     解决：进入menuconfig界面-component config-ESP-TLS 勾选
 *          Allow potentially insecure options
 *          Skip server certificate verification by default (WARNING: ONLY FOR TESTING PURPOSE, READ HELP)
 */

static const char *TAG = "http_weather";
static char http_response_body[MAX_HTTP_OUTPUT_BUFFER] = {0};

//json 数据解析
static cjson_region_info_t cjson_region_info;
static cjson_weather_info_t cjson_weather_info;
static cjson_date_info_t cjson_date_info;

//日期
static Date_t now_date;

/**
 * @brief 解析城市json数据
 * @param *message, json数据
 * @param *output, 解析的json数据通过指针的方式赋值, 函数类型请查看 cjson_region_info_t
 * @return 如果解析失败返回-1, 否则返回0
 */
static int city_cjson_to_struct_info(char *message, cjson_region_info_t *output)
{
#if 0
/*
    http://ip-api.com/json/ -- 服务器异常
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
    /* 解析整段json数据 */
    output->cjson_root = cJSON_Parse(message);

    if(output->cjson_root == NULL)
    {
        printf("city json parse fail.\n");
        printf("Error before: [%s]\n",cJSON_GetErrorPtr());
        return -1;
    }

    printf("city json parse succeed.\n");

    /* 解析数据 */
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
        "country": "中国", 
        "short_name": "CN", 
        "province": "广东省", 
        "city": "深圳市", 
        "area": "龙岗区", 
        "isp": "天威视讯", 
        "net": "", 
        "ip": "115.45.57.209",
        "code": 200, 
        "desc": "success"
    }
*/

    /* 解析整段json数据 */
    output->cjson_root = cJSON_Parse(message);

    if(output->cjson_root == NULL)
    {
        printf("city json parse fail.\n");
        return -1;
    }

    /* 解析数据 */
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
 * @brief 解析天气json数据
 * @param *message, json数据
 * @param *output, 解析的json数据通过指针的方式赋值, 函数类型请查看 cjson_weather_info_t
 * @return 如果解析失败返回-1, 否则返回0
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
                    "name":"深圳",
                    "country":"CN",
                    "path":"深圳,深圳,广东,中国",
                    "timezone":"Asia/Shanghai",
                    "timezone_offset":"+08:00"
                },

                "now":
                {
                    "text":"阴",
                    "code":"9",
                    "temperature":"27"
                },

                "last_update":"2022-07-03T23:12:40+08:00"
            }
        ]
    }
*/

    cJSON *object;

    /* 解析整段json数据 */
    output->root = cJSON_Parse(message);

    if(output->root == NULL)
    {
        printf("city json parse fail.\n");
        return -1;
    }

    printf("weather json parse succeed.\n");

    /* 获取列表数据 */
    output->cjson_arrayItem = cJSON_GetObjectItem(output->root, "results");

    /* 解析results内部的列表数据 */
    if(output->cjson_arrayItem != NULL)
    {
        /* 获取列表长度 */
        int size = cJSON_GetArraySize(output->cjson_arrayItem);
        printf("cJSON_GetArraySize: size=%d\n", size);

        for (int i = 0; i < size; i++)
        {
            printf("i=%d\n", i);
            object = cJSON_GetArrayItem(output->cjson_arrayItem, i);

            //从列表中获取 location 对象
            output->cjson_location = cJSON_GetObjectItem(object, "location");
            if(output->cjson_location == NULL)
            {
                printf("city json parse [location] fail.\n");
                return -1;
            }
            //解析 location 数据
            output->cjson_location_id = cJSON_GetObjectItem(output->cjson_location, "id");
            output->cjson_location_name = cJSON_GetObjectItem(output->cjson_location, "name");
            output->cjson_location_country = cJSON_GetObjectItem(output->cjson_location, "country");
            output->cjson_location_path = cJSON_GetObjectItem(output->cjson_location, "path");
            output->cjson_location_timezone = cJSON_GetObjectItem(output->cjson_location, "timezone");
            output->cjson_location_timezone_offset = cJSON_GetObjectItem(output->cjson_location, "timezone_offset");

            //从列表中获取 now 对象
            output->cjson_now = cJSON_GetObjectItem(object, "now");
            if(output->cjson_now == NULL)
            {
                printf("city json parse [now] fail.\n");
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                return -1;
            }
            //解析 now 数据
            output->cjson_now_text = cJSON_GetObjectItem(output->cjson_now, "text");
            output->cjson_now_code = cJSON_GetObjectItem(output->cjson_now, "code");
            output->cjson_now_temperature = cJSON_GetObjectItem(output->cjson_now, "temperature");

            //从列表中获取 last_update 对象
            output->cjson_last_update = cJSON_GetObjectItem(object, "last_update");
            if(output->cjson_last_update == NULL)
            {
                printf("city json parse [last_update] fail.\n");
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                return -1;
            }
            //解析 last_update 对象
            output->cjson_last_update = cJSON_GetObjectItem(output->cjson_last_update, "last_updata");


            printf("weather:%s\n", output->cjson_now_text->valuestring);
        }
    }

    return 0;
}


/**
 * @brief 解析日期json数据
 * @param *message, json数据
 * @param *output, 解析的json数据通过指针的方式赋值, 函数类型请查看 cjson_weather_info_t
 * @return 如果解析失败返回-1, 否则返回0
 */
static int date_cjson_to_struct_info(char *message, cjson_date_info_t *output)
{
/*
    {
        "sysTime2":"2022-08-12 22:46:20",
        "sysTime1":"20220812224620"
    }
*/
    /* 解析整段json数据 */
    output->root = cJSON_Parse(message);

    if(output->root == NULL)
    {
        printf("date json parse fail.\n");
        return -1;
    }

    printf("date json parse succeed.\n");

    /* 解析数据 */
    output->sysTime1 = cJSON_GetObjectItem(output->root, "sysTime1");
    output->sysTime2 = cJSON_GetObjectItem(output->root, "sysTime2");

    return 0;
}

/**
 * @brief 解析字符串时间
 * @param str_data 待解析的日期字符串
 * @param now_date 储存解析的日期
 * @return 成功返回0, 失败返回-1
 * @note 例如解析日期字符串 "20220812095735", 此函数判断年份范围(1970年-2100年)
 */
int http_analysis_str_date(char *str_data, Date_t *now_date)
{
    int ret = 0;
    uint16_t year=0, month=0, day=0, hour=0, min=0, sec=0;
    char *p_tmp;
    
    p_tmp = str_data;

    //样例字符串, 用来判断输入字符是否合规
    uint8_t str_example[] = "20220812095735";
    uint8_t str_example_len, str_len = 0;

    //样例字符串长度, 或者使用 strlen(str_example) 就不用减1
    str_example_len = sizeof(str_example) - 1; 

    //判断长度是否正确
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
    
    //解析日期
    for (int i = 0; i < str_example_len; i++)
    {
        printf("%c:%d\n", str_data[i], str_data[i]);
        //0-3字节 - 年份
        if(i<4)
        {
            year += pow(10, (3-i)) * (str_data[i] - '0');
        }
        //4-5字节 - 月
        else if(i<6)
        {
            month +=  pow(10, (5-i)) * (str_data[i] - '0');
        }
        //6-7 - 日
        else if(i<8)
        {
            day +=  pow(10, (7-i)) * (str_data[i] - '0');
        }
        //8-9字节 - 时
        else if(i<10)
        {
            hour +=  pow(10, (9-i)) * (str_data[i] - '0');
        }
        //10-11字节 - 分
        else if(i<12)
        {
            min +=  pow(10, (11-i)) * (str_data[i] - '0');
        }
        //12-13字节 - 秒
        else if(i<14)
        {
            sec +=  pow(10, (13-i)) * (str_data[i] - '0');
        }
        else
        {
            return -1;
        }
    }

    //年份和月份判断
    if(year < 1970 || year > 2100 ||month == 0 || month > 12 || hour >= 24 || min >= 60 || sec >= 60)
    {
        return -1;
    }

    //day判断
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
            //闰年判断, 能被4整除却不能被100整除 或 能被400整除的年份就是闰年
            if((year%4 == 0 && year % 100 != 0) || year%400 == 0)
            {
                //闰年, 2月29天
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

    //数据解析成功
    now_date->year = year;
    now_date->month = month;
    now_date->day = day;

    now_date->hour = hour;
    now_date->minute = min;
    now_date->second = sec;

    return ret;
}


/**
 * @brief 通过http请求获取JSON数据, JSON数据最大长度参考宏 MAX_HTTP_OUTPUT_BUFFER。
 *        由于http_response_body是全局变量，如果是在线程中调用此函，需添加互斥锁，保护临界资源
 * @param *url, http请求链接
 * @return 如果获取json数据失败返回-1, 否则返回0
 */
static int http_get_json(char *url)
{
    //char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   //用于接收通过http协议返回的数据
    int content_length = 0;  //http协议头的长度

    //线程中调用此函数，在此处添加互斥锁
    memset(http_response_body, 0, sizeof(http_response_body));

    //02-2 配置http结构体
    //定义http配置结构体，并且进行清零
    esp_http_client_config_t config ;
    memset(&config,0,sizeof(config));

    //向配置结构体内部写入url
    config.url = url;

    //初始化结构体
    esp_http_client_handle_t client = esp_http_client_init(&config);	//初始化http连接

    //设置发送请求
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    //与目标主机创建连接，并且声明写入内容长度为0
    //因为如果是post请求，会在报文的头部后面跟着要向服务器发送的数据
    //而对于get方法，发送的内容都在URL里面，都在报文头部，不需要定义后面的部分，因此写入长度就是0
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK)
    {
        //连接失败
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return -1;
    }
    else
    {
        //连接成功
        //读取目标主机的返回内容的协议头
        content_length = esp_http_client_fetch_headers(client);

        //如果协议头长度小于0，说明没有成功读取到
        if (content_length < 0)
        {
            ESP_LOGE(TAG, "HTTP client fetch headers failed");
        }
        else
        {
            //读取目标主机通过http的响应内容
            int data_read = esp_http_client_read_response(client, http_response_body, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
                //打印响应内容，包括响应状态，响应体长度及其内容
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),				//获取响应状态信息
                esp_http_client_get_content_length(client));			//获取响应信息长度
                printf("data:%s\n", http_response_body);
            }
            else
            {
                //读取响应体失败
                ESP_LOGE(TAG, "Failed to read response");
            }
        }
    }

    //关闭连接
    esp_http_client_close(client);
    return 0;
}


/**
 * @brief http获取城市JSON数据，并解析数据
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
 * @brief http获取天气JSON数据，并解析
 * @param none
 * @return 如果获取天气数据失败返回-1, 否则返回0
 */
int http_get_weather_info(void)
{
    /*
        1.拼接字符串
        "https://api.seniverse.com/v3/weather/now.json?key=" + ZHIXIN_WEATHER_KEY + "&location=" + cjson_city->valuestring + "&language=zh-Hans&unit=c"
        
        拼接后："https://api.seniverse.com/v3/weather/now.json?key=Sb6FCTaiQBJGHuv98&location=beijing&language=zh-Hans&unit=c"
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
        /* 解析城市json数据 */
        json_analysis_ret = city_cjson_to_struct_info(http_response_body, &cjson_region_info);

        if(json_analysis_ret != -1)
        {
            /* 如果城市数据解析成功，开始拼接字符串 */
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
                    //天气数据解析成功
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
        printf("获取城市json数据失败\n");
    }

    return result;
}



/**
 * @brief http获取日期JSON数据，并解析数据
 * @param none
 * @return 如果获取天气数据失败返回-1, 否则返回0
 */
int http_get_date_info(void)
{
    int ret = 0;
    int response_state = http_get_json( (char *)BEIJING_TIME_LINK );
    if(response_state == -1)
    {
        return -1;
    }

    //获取数据成功
    ret = date_cjson_to_struct_info(http_response_body, &cjson_date_info);
    if(ret == -1)
        return -1;
    
    //将字符串日期转换为数字日期 - 年月日时分秒
    ret = http_analysis_str_date(cjson_date_info.sysTime2->valuestring, &now_date);
    if(ret == -1)
        return -1;

    //获取星期 
    now_date.week = get_Week(now_date.year, now_date.month, now_date.day);

    return 0;
}

void http_weater_test(void)
{
    http_get_weather_info();
}
