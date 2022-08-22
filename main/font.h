#pragma once

//�ַ������
#define FONT_STRING_WIDTH     16
#define FONT_STRING_HEIGHT    32

//���ֿ��
#define FONT_CHINESE_WIDTH    32
#define FONT_CHINESE_HEIGHT   32

//ͼ����
#define ICON_WIDTH            56
#define ICON_HEIGHT           56

//���ָ��� -- ���/ɾ�������ֿ�ʱ, ��Ҫ�޸Ĵ�ֵ
#define CHINESE_GBK_NUM      10  //sizeof(Chinese_Gbk)����, ʹ�ú궨���ȡ�ֿ�����

//�ļ������ʽ
#define FILE_CODING_TYPE_UTF_8
//#define FILE_CODING_TYPE_GB2312


//��������
#define FONT_ARRAY_SIZE  (FONT_CHINESE_WIDTH/8*FONT_CHINESE_HEIGHT)
typedef struct typFNT_GB2312
{
    unsigned char Index[2];	
    char Msk[FONT_ARRAY_SIZE];
}typFNT_GB2312_t;


//ͼ������
typedef enum
{
    LCD_ICON_INDEX_END,  //������־
}Lcd_Icon_Index_e;

extern typFNT_GB2312_t Chinese_Gbk[];
extern unsigned char ascii_3216[];
extern unsigned char ascii_1608[];
extern unsigned char weather_icon_tab[][ICON_WIDTH/8*ICON_HEIGHT];
