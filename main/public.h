#pragma once
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

//星期
typedef enum _week
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday,
}Week_e;

//日期信息
typedef struct _date
{
    uint16_t year;
    uint8_t  month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    Week_e week;  //0-6(周一至周日)
}Date_t;

int get_Week(int y, int m, int d);
