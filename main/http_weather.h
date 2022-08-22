#pragma once
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "public.h"

//http response max len
#define MAX_HTTP_OUTPUT_BUFFER   1024

#if 0
    //city data url -- 服务器异常
    #define REGION_HRTTPS_LINK       "http://ip-api.com/json/" 
#else
    #define REGION_HRTTPS_LINK       "https://ip.useragentinfo.com/json" 
#endif

//心知天气key
#define XINZHI_WEATHER_KEY       "Sb6FCTaiQBJGHuv98"

//北京时间 -- 校准时间使用
#define BEIJING_TIME_LINK        "http://quan.suning.com/getSysTime.do"

#if 0
//http://ip-api.com/json/ 服务器异常, 所以不用
typedef struct 
{
    cJSON* cjson_root;
    cJSON* cjson_status;
    cJSON* cjson_country;
    cJSON* cjson_countryCode;
    cJSON* cjson_region;
    cJSON* cjson_regionName;
    cJSON* cjson_city;
    cJSON* cjson_zip;
    cJSON* cjson_lat;
    cJSON* cjson_lon;
    cJSON* cjson_timezone;
    cJSON* cjson_isp;
    cJSON* cjson_org;
    cJSON* cjson_as;
    cJSON* cjson_query;
}cjson_region_info_t;
#else
//"https://ip.useragentinfo.com/json"
typedef struct 
{
    cJSON* cjson_root;
    cJSON* cjson_country;
    cJSON* cjson_short_name;
    cJSON* cjson_province;
    cJSON* cjson_city;
    cJSON* cjson_area;
    cJSON* cjson_isp;
    cJSON* cjson_net;
    cJSON* cjson_ip;
    cJSON* cjson_code;
    cJSON* cjson_desc;
}cjson_region_info_t;
#endif

//心知天气
typedef struct 
{
    cJSON* root;       
    cJSON* cjson_arrayItem;   
    cJSON* cjson_results_data;  

    cJSON* cjson_location; 
    cJSON* cjson_location_id;
    cJSON* cjson_location_name;
    cJSON* cjson_location_country;
    cJSON* cjson_location_path;
    cJSON* cjson_location_timezone;
    cJSON* cjson_location_timezone_offset;

    cJSON* cjson_now; 
    cJSON* cjson_now_text; 
    cJSON* cjson_now_code;
    cJSON* cjson_now_temperature;

    cJSON* cjson_last_update; 
}cjson_weather_info_t;


typedef struct _beijing_time
{
    cJSON* root;
    cJSON* sysTime1;
    cJSON* sysTime2;
}cjson_date_info_t;


void http_weater_test(void);
