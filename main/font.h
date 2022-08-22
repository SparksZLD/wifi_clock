#pragma once

//字符串宽高
#define FONT_STRING_WIDTH     16
#define FONT_STRING_HEIGHT    32

//汉字宽高
#define FONT_CHINESE_WIDTH    32
#define FONT_CHINESE_HEIGHT   32

//图标宽高
#define ICON_WIDTH            56
#define ICON_HEIGHT           56

//汉字个数 -- 添加/删除中文字库时, 需要修改此值
#define CHINESE_GBK_NUM      10  //sizeof(Chinese_Gbk)报错, 使用宏定义获取字库数量

//文件编码格式
#define FILE_CODING_TYPE_UTF_8
//#define FILE_CODING_TYPE_GB2312


//汉字类型
#define FONT_ARRAY_SIZE  (FONT_CHINESE_WIDTH/8*FONT_CHINESE_HEIGHT)
typedef struct typFNT_GB2312
{
    unsigned char Index[2];	
    char Msk[FONT_ARRAY_SIZE];
}typFNT_GB2312_t;


//图标索引
typedef enum
{
    LCD_ICON_INDEX_END,  //结束标志
}Lcd_Icon_Index_e;

extern typFNT_GB2312_t Chinese_Gbk[];
extern unsigned char ascii_3216[];
extern unsigned char ascii_1608[];
extern unsigned char weather_icon_tab[][ICON_WIDTH/8*ICON_HEIGHT];
