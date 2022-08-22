#include "hal_lcd.h"

/* ���ļ�ʵ����Ļ���ܺ����� */

/**
 * @brief ��������Ƿ�Ϲ�
 * @param start_x - x��
 * @param start_y - y��
 * @param width - ��
 * @param height - ��
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_check_xy_area(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height)
{
    /* �жϸ������Ƿ�����ʾһ���ַ� */
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //�������, ֱ���˳�
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    if( (start_x > (LCD_WIDTH - (width-1)) ) || (start_y > (LCD_HEIGHT - (height-1))))
    {
        //�����Ļ
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if(width == 0 || height == 0)
    {
        //��������
        return LCD_DISPLAY_AREA_ERR;
    }

    return LCD_DISPLAY_OK;
}




/**
 * @brief ��ʾ��������
 * @param start_x - x��
 * @param start_y - y��
 * @param font_color - ������ɫ
 * @param back_color - ������ɫ
 * @param p_str - �����ַ�, ע��ʹ�ñ����ʽ��gb2312, ����ռ2���ֽ�
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_chinese(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_str)
{
    int i=0, font_num=0;
    uint8_t f_buf[2], b_buf[2];
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //Ĭ�����ҵ�����

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //λ���ж�
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //��ַ����, ֱ���˳�
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* �жϸ������Ƿ�����ʾ�����ĺ��� */
    if( (start_x > LCD_WIDTH - FONT_CHINESE_WIDTH) || (start_y > LCD_HEIGHT - FONT_CHINESE_HEIGHT))
    {
        //�������, ֱ���˳�
        return LCD_DISPLAY_OVERFLOW_ERR;
    }
    else
    {
        if(p_str[0] < 128)
        {
            //�������ĺ���
            ret = LCD_DISPLAY_NOT_FOUND_ERR;
        }
        else
        {
            //��ȡ�ֿ⺺������
            //font_num = sizeof(Chinese_Gbk) / sizeof(typFNT_GB2312_t); //sizeof�޷�ʹ��, ʹ�ú궨���ȡ�ֿ�����
            font_num = CHINESE_GBK_NUM; //���鳤��
            //printf("font_num = %d\n", font_num);

            for (i = 0; i < font_num; i++)
            {
                if(Chinese_Gbk[i].Index[0] == p_str[0] && Chinese_Gbk[i].Index[1] == p_str[1])
                {
                    break;
                }
            }

            if(i == font_num)
            {
                //û�ҵ�������
                ret = LCD_DISPLAY_NOT_FOUND_ERR;
                printf("chinese buf not found chinese array\n");
            }
            else
            {
                //�ҵ�����, ��������
                lcd_address_window_set(start_x, start_y, start_x + FONT_CHINESE_WIDTH-1, start_y + FONT_CHINESE_HEIGHT-1);

                //��ʾ����
                for (int j = 0; j < FONT_ARRAY_SIZE; j++)
                {
                    for (int k = 0; k < 8; k++)
                    {
                        if(Chinese_Gbk[i].Msk[j] & (0x80>>k))
                        {
                            st7735_data(f_buf, 2);
                        }
                        else
                        {
                           st7735_data(b_buf, 2);
                        }
                    }
                }
            }
        }
    }

    return ret;
}


/**
 * @brief ��ʾ�����ַ�(32*16)
 * @param start_x - x��
 * @param start_y - y��
 * @param font_color - ������ɫ
 * @param back_color - ������ɫ
 * @param p_char - Ӣ���ַ�
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_char(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_char)
{
    uint8_t f_buf[2], b_buf[2], tmp=0;
    int index=0, char_array_size=0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //Ĭ������

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //λ���ж�
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //��ַ����, ֱ���˳�
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* �жϸ������Ƿ�����ʾһ���ַ� */
    if( (start_x > LCD_WIDTH - FONT_STRING_WIDTH) || (start_y > LCD_HEIGHT - FONT_STRING_HEIGHT))
    {
        //�������, ֱ���˳�
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if((*p_char > 128) || (*p_char<32))
    {
        //����Ӣ���ַ�
        ret = LCD_DISPLAY_NOT_FOUND_ERR;
    }
    else
    {
        //��ȡ�����ַ�, �����С
        char_array_size = FONT_STRING_WIDTH/8*FONT_STRING_HEIGHT;

        //��ȡ��ʼ��ַ
        index = (*p_char - 32) * char_array_size;

        //��������
        lcd_address_window_set(start_x, start_y, start_x+FONT_STRING_WIDTH-1, start_y+FONT_STRING_HEIGHT-1);

        for (int i = 0; i < char_array_size; i++)
        {
            tmp = ascii_3216[index + i];

            for (int j = 0; j < 8; j++)
            {
                if(tmp & (0x80>>j))
                {
                    st7735_data(f_buf, 2);
                }
                else
                {
                    st7735_data(b_buf, 2);
                }
            }
        }
    }

    return ret;
}



/**
 * @brief ��ʾ�����ַ�(16*88)
 * @param start_x - x��
 * @param start_y - y��
 * @param font_color - ������ɫ
 * @param back_color - ������ɫ
 * @param p_char - Ӣ���ַ�
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_char_1608(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_char)
{
    uint8_t f_buf[2], b_buf[2], tmp=0;
    int index=0, char_array_size=0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //Ĭ������

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //λ���ж�
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //��ַ����, ֱ���˳�
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* �жϸ������Ƿ�����ʾһ���ַ� */
    if( (start_x > LCD_WIDTH - 8 +1) || (start_y > LCD_HEIGHT - 16+1))
    {
        //�������, ֱ���˳�
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if((*p_char > 128) || (*p_char<32))
    {
        //����Ӣ���ַ�
        ret = LCD_DISPLAY_NOT_FOUND_ERR;
    }
    else
    {
        //��ȡ�����ַ�, �����С
        char_array_size = 16;

        //��ȡ��ʼ��ַ
        index = (*p_char - 32) * char_array_size;

        //��������
        lcd_address_window_set(start_x, start_y, start_x+8-1, start_y+16-1);

        for (int i = 0; i < char_array_size; i++)
        {
            tmp = ascii_1608[index + i];

            for (int j = 0; j < 8; j++)
            {
                if(tmp & (0x80>>j))
                {
                    st7735_data(f_buf, 2);
                }
                else
                {
                    st7735_data(b_buf, 2);
                }
            }
        }
    }

    return ret;
}


/**
 * @brief ��ʾ16*8�ַ���
 * @param start_x - x��
 * @param start_y - y��
 * @param font_color - ������ɫ
 * @param back_color - ������ɫ
 * @param p_str - �ַ���
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_draw_string_1608(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_str)
{
    uint16_t x, y;
    uint8_t *tmp;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    tmp = p_str;

    x = start_x;
    y = start_y;

    while(*tmp)
    {
        if(*tmp == '\n')
        {
            //����
            y += 16;
            x = 0;
            tmp += 1;
            continue;
        }
        //printf("tmp = %d\n", tmp[0]);
        ret = hal_lcd_draw_char_1608(x, y, font_color, back_color, tmp);

        switch (ret)
        {
            case LCD_DISPLAY_OK:
                //�ַ���ʾ�ɹ�
                x += 8;
                tmp += 1;
                break;

            default:
                return ret;
        }
    }
    return LCD_DISPLAY_OK;
}



/**
 * @brief ��ʾ��Ӣ���ַ���
 * @param start_x - x��
 * @param start_y - y��
 * @param font_color - ������ɫ
 * @param back_color - ������ɫ
 * @param p_str - �ַ���
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 * @note ָ��λ����ʾ��Ӣ���ַ��� ����32*32, Ӣ��16*32, �����ַ�����gb2312����
 */
Lcd_Diplay_State_e hal_lcd_draw_font(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_str)
{
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;
    uint16_t x, y;
    uint8_t *tmp;

    tmp = p_str;

    x = start_x;
    y = start_y;

    while(*tmp)
    {
        if(*tmp == '\n')
        {
            //����
            y += FONT_STRING_HEIGHT;
            x = 0;
            tmp += 1;
            continue;
        }
        //printf("tmp = %d\n", tmp[0]);
        ret = hal_lcd_draw_char(x, y, font_color, back_color, tmp);

        switch (ret)
        {
            case LCD_DISPLAY_OK:
                //�ַ���ʾ�ɹ�
                x += FONT_STRING_WIDTH;
                tmp += 1;
                break;

            case LCD_DISPLAY_NOT_FOUND_ERR:
                //û�ҵ��ַ��������������ַ�
                ret = hal_lcd_draw_chinese(x, y, font_color, back_color, tmp);

                switch(ret)
                {
                    case LCD_DISPLAY_OK:
                    case LCD_DISPLAY_NOT_FOUND_ERR:
                        x += FONT_CHINESE_WIDTH;
                        tmp += 2;
                        break;

                    case LCD_DISPLAY_ADDRESS_ERR:  //��ַ����
                    case LCD_DISPLAY_OVERFLOW_ERR: //�����Ļ
                        //�����ַ����, ֱ���˳�
                        printf("chinese addr err\n");
                        return LCD_DISPLAY_ADDRESS_ERR;

                    default:
                        break;
                }
                break;

            case LCD_DISPLAY_ADDRESS_ERR:  //��ַ����
            case LCD_DISPLAY_OVERFLOW_ERR: //�����Ļ
                //�����ַ����, ֱ���˳�
                printf("char addr err\n");
                return LCD_DISPLAY_ADDRESS_ERR;
                //break;

            default:
                break;
        }
    }

    return ret;
}


/**
 * @brief ָ��������ʾͼƬ
 * @param start_x - x��
 * @param start_y - y��
 * @param width - ��������
 * @param height - ��������
 * @param fill_color - ���ͼ������ɫ
 * @param back_color - ������ɫ
 * @param p_buf - ������ʼ��ַ
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_fill_picture(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Lcd_Color_e fill_color, Lcd_Color_e back_color, uint8_t *p_buf)
{
    uint8_t f_buf[2], b_buf[2];
    uint16_t buf_size = 0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    f_buf[0] = (uint8_t)(fill_color>>8);
    f_buf[1] = (uint8_t)(fill_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //���ش�����Ϣ
        return ret;
    }

    //��������
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    buf_size = width/8*height;

    /* ˢ��ͼ�� */
    for (int i = 0; i <buf_size ; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if(*(p_buf+i) & (0x80>>j))
            {
                st7735_data(f_buf, 2);
            }
            else
            {
                st7735_data(b_buf, 2);
            }
        }
    }

    return LCD_DISPLAY_OK;
}



/**
 * @brief ��һ����
 * @param start_x - x��
 * @param start_y - y��
 * @param width - �߶ο�
 * @param height - �߶θ�
 * @param line_color - �߶���ɫ
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_draw_line(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t  height, Lcd_Color_e line_color)
{
    uint8_t line_buf[2];
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    line_buf[0] = (uint8_t)(line_color>>8);
    line_buf[1] = (uint8_t)(line_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //���ش�����Ϣ
        return ret;
    }

    //��������
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    for (int i = 0; i < width*height; i++)
    {
        st7735_data(line_buf, 2);
    }

    return LCD_DISPLAY_OK;
}


/**
 * @brief ��ʾ���ں�����
 * @param x - ������
 * @param y - ������
 * @param year - ��
 * @param month - ��
 * @param day - ��
 * @param week - ����, ������鿴 Week_e
 * @return none
 */
void hal_lcd_show_Date(uint16_t x, uint16_t y, uint16_t year, uint16_t month, uint16_t day, Week_e week)
{
    uint8_t str_buf[30]; //�洢�����ַ���

    //x = LCD_DATE_SHOW_X;
    //y = LCD_DATE_SHOW_Y;

    //�������� - ռ��11���ֽ�, ʣ�����ڲ��ֳ�ʼ��Ϊ�ո�
    sprintf((char *)str_buf, "%4d-%02d-%02d     ", year, month, day);

    //�������� -- ռ��2���ֽ�
    switch(week)
    {
        case Monday:
            sprintf((char *)&str_buf[11], "��%s", "һ");
            break;
        case Tuesday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        case Wednesday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        case Thursday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        case Friday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        case Saturday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        case Sunday:
            sprintf((char *)&str_buf[11], "��%s", "��");
            break;
        default:
            break;
    }

    hal_lcd_draw_font(x, y, LCD_DATE_SHOW_COLOR, LCD_DATE_SHOW_BACK_COLOR, str_buf);
}


/**
 * @brief ��ʾʱ��
 * @param x - ������
 * @param y - ������
 * @param hour - ʱ
 * @param min - ��
 * @param sec - ��
 * @return none
 */
void hal_lcd_show_time(uint16_t x, uint16_t y, uint8_t hour, uint8_t min, uint8_t sec)
{
    uint8_t str_buf[30];

    //������ʾλ��
    // x = LCD_TIME_SHOW_X;
    // y = LCD_TIME_SHOW_Y;

    sprintf((char *)&str_buf[0], "%02d:%02d:%02d", hour, min, sec);

    //��ʾ
    hal_lcd_draw_font(x, y, LCD_TIME_SHOW_COLOR, LCD_TIME_SHOW_BACK_COLOR, str_buf);
}

/**
 * @brief ��ʾʱ��(С����)
 * @param x - ������
 * @param y - ������
 * @param hour - ʱ
 * @param min - ��
 */
void hal_lcd_show_small_time(uint16_t x, uint16_t y, uint8_t hour, uint8_t min)
{
    uint8_t buf[30] = {0};
    uint16_t font_color, back_color;

    // x = LCD_WEATHER_UPDATE_SHOW_X;
    // y = LCD_WEATHER_UPDATE_SHOW_Y;

    font_color = LCD_SMALL_TIME_SHOW_COLOR;
    back_color = LCD_SMALL_TIME_SHOW_BACK_COLOR;

    sprintf((char *)buf, "%02d:%02d", hour, min);

    hal_lcd_draw_string_1608(x, y, font_color, back_color, buf);
}

/**
 * @brief ��ʾ�¶ȣ���������
 * @param x - ������
 * @param y - ������
 * @param temperature - �¶�
 * @note 1.��ʾ��Χ -99�� - 99��, ����ֻ��ʾ����
 */
void hal_lcd_show_temperature(uint16_t x, uint16_t y, int temperature)
{
    uint8_t buf[30] = {0};
    uint8_t du[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,
    0x08,0x10,0x08,0x10,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* �� */
    };

    if(temperature < -99 || temperature > 99)
        return;

    if(temperature > -100 && temperature < -9)
    {
        //ռ��3���ֽ�
        sprintf((char *)buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //�¶�ռ��1���ֽ�
        sprintf((char *)buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)buf, " %d", temperature);
    }
    
    //ˢ���¶�
    hal_lcd_draw_font(x, y, COLOR_WHITE, COLOR_BLACK, (uint8_t *)buf);
    hal_lcd_fill_picture(x + 48, y, 16, 32, COLOR_WHITE, COLOR_BLACK, du);
    hal_lcd_draw_font(x + 64, y, COLOR_WHITE, COLOR_BLACK, (uint8_t *)"C");
}


/**
 * @brief ��ʾ�¶ȣ������С16*8
 * @param x - ������
 * @param y - ������
 * @param temperature - �¶�
 * @note 1.��ʾ��Χ -99�� - 99��, ����ֻ��ʾ����
 */
void hal_lcd_show_Small_temperature(uint16_t x, uint16_t y, int temperature)
{
    uint8_t buf[30] = {0};
    uint8_t du[] = {0x00,0x00,0x18,0x24,0x24,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"��",0*/};

    // x = LCD_TEMP_SMALL_SHOW_X;
    // y = LCD_TEMP_SMALL_SHOW_Y;

    if(temperature < -99 || temperature > 99)
        return;

    if(temperature > -100 && temperature < -9)
    {
        //ռ��3���ֽ�
        sprintf((char *)buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //�¶�ռ��1���ֽ�
        sprintf((char *)buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)buf, " %d", temperature);
    }

    //ˢ���¶�
    hal_lcd_draw_string_1608(x, y, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, (uint8_t *)buf);
    hal_lcd_fill_picture(x + 24, y, 8, 16, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, du);
    hal_lcd_draw_string_1608(x + 32, y, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, (uint8_t *)"C");
}


/**
 * @brief ָ��������ʾͼƬ, ˢ��һ��, ����һ��, �˺���Ŀǰֻ�� hal_lcd_slide_show �������õ�
 * @param start_x - x��
 * @param start_y - y��
 * @param width - ��������
 * @param height - ��������
 * @param fill_color - ���ͼ������ɫ
 * @param back_color - ������ɫ
 * @param p_buf - ������ʼ��ַ
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e, 
 */
Lcd_Diplay_State_e hal_lcd_fill_picture_jump(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Lcd_Color_e fill_color, Lcd_Color_e back_color, uint8_t *p_buf)
{
    uint8_t f_buf[2], b_buf[2];
    uint16_t buf_size = 0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;
    uint8_t *tmp = p_buf;

    //��16bit��ɫ����ת��Ϊ����, �������ʹ��
    f_buf[0] = (uint8_t)(fill_color>>8);
    f_buf[1] = (uint8_t)(fill_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //���ش�����Ϣ
        return ret;
    }

    //��������
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    buf_size = width/8*height;

    //һ��һ��ˢ��
    for (int i = 0; i < buf_size; i++)
    {
        //ÿһ���������ʼλ��
        if(i % (width/8) == 0 && i/(width/8) != 0)
        {
            tmp += width/8;
        }

        //ˢ��һ������
        for (int j = 0; j < 8; j++)
        {
            if(*(tmp+i) & (0x80>>j))
            {
                st7735_data(f_buf, 2);
            }
            else
            {
                st7735_data(b_buf, 2);
            }
        }
    }
    
    return LCD_DISPLAY_OK;
}

/**
 * @brief �õ�Ƭ(��ɫ)
 * @param Lcd_Slide_Config - ����������鿴 Lcd_Slide_Config_t
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 * @note 1.�˺�������ѭ����ʱ�����鴴��������ô˺���
 *       2.�õ�Ƭ����Ŀ���Ҫ��8�ı���
 *       3.���ˢ�·������������, ��ô����������8�ı���, ��Ϊ���ⲽ��ʵ�ֱȽ��鷳, �Ҳ���д
 *       4.Ŀǰֻ��ˢ��ԭͼ��Ŀ��ͼ, ��ɫһ�µ�ͼƬ, ��ͬ��ɫˢ����Ҳ����д
 */
Lcd_Diplay_State_e hal_lcd_slide_show(Lcd_Slide_Config_t Lcd_Slide_Config)
{
    uint16_t frame_num=0; //֡��
    uint16_t start_x, start_y, width, height; //�õ�Ƭ����
    uint16_t src_color, src_back, dest_color, dest_back; //��ɫ
    uint16_t step_len; //����
    uint8_t  *buf; 
    uint8_t  color_same_flag = 0; 

    uint16_t buf_offset = 0; //bufƫ�Ʋ���

    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    start_x = Lcd_Slide_Config.x;
    start_y = Lcd_Slide_Config.y;
    width = Lcd_Slide_Config.width;
    height = Lcd_Slide_Config.height;

    src_color = Lcd_Slide_Config.src_color;
    src_back = Lcd_Slide_Config.src_back_color;
    dest_color = Lcd_Slide_Config.dest_color;
    dest_back = Lcd_Slide_Config.dest_back_color;

    //����
    step_len = Lcd_Slide_Config.step_len;

    //�����ж�
    if(step_len == 0)
    {
        //��������
        return LCD_DISPLAY_SLIDE_STEP_ERR;
    }
    if(Lcd_Slide_Config.slide_dir == Lcd_Slide_Left || Lcd_Slide_Config.slide_dir == Lcd_Slide_Right)
    {
        if( step_len%8 != 0)
            return LCD_DISPLAY_SLIDE_STEP_ERR;
    }

    //�����ж�
    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if (ret != LCD_DISPLAY_OK)
    {
        return ret;
    }

    //����ԭͼ2���ռ�
    buf = (uint8_t *)malloc( width/8*height * 2 );

    //�ж�ԭͼ��Ŀ��ͼƬ��ɫ �� ����ɫ�Ƿ�һ��
    if(src_color == dest_color && src_back == dest_back)
    {
        color_same_flag = 1;
    }

    switch(Lcd_Slide_Config.slide_dir)
    {
        case Lcd_Slide_Top:
            //�����ƶ�, src_buf ������� dest_buf ����
            //���� src_buf �� buf ��
            memcpy(buf, Lcd_Slide_Config.src_buf, width/8 * height);

            //���� dest_buf �� buf ��
            memcpy(buf + width/8*height, Lcd_Slide_Config.dest_buf, width/8 * height);
            break;
        case Lcd_Slide_Bottom:
            //�����ƶ�, dest_buf ������� src_buf ����
            //���� dest_buf �� buf ��
            memcpy(buf + width/8*height, Lcd_Slide_Config.src_buf, width/8 * height);

            //���� src_buf �� buf ��
            memcpy(buf, Lcd_Slide_Config.dest_buf, width/8 * height);
            break;
        case Lcd_Slide_Left:
            //�����ƶ�, src_buf �� dest_buf ���н�����
            for (int i = 0; i < height; i++)
            {
                //���� src_buf
                memcpy(buf + 2*i*width/8, Lcd_Slide_Config.src_buf + i*width/8, width/8);

                //���� dest_buf
                memcpy(buf + (2*i+1)*width/8 , Lcd_Slide_Config.dest_buf + i*width/8, width/8);
            }

            break;
        case Lcd_Slide_Right:
            //�����ƶ�, dest_buf �� src_buf ���н�����
            for (int i = 0; i < height; i++)
            {
                //���� dest_buf 
                memcpy(buf + 2*i*width/8, Lcd_Slide_Config.dest_buf + i*width/8, width/8);

                //���� src_buf
                memcpy(buf + (2*i+1)*width/8 , Lcd_Slide_Config.src_buf + i*width/8, width/8);
            }
            break;
        default:
            return LCD_DISPLAY_SLIDE_DIR_ERR; //���ط������
    }
    
    if(color_same_flag)
    {
        //�����ɫһ��, src �� buf ���÷ֿ�ˢͼ
        switch (Lcd_Slide_Config.slide_dir)
        {
            case Lcd_Slide_Top:
                frame_num = height; //������ˢ�µ�֡��
                while (frame_num)
                {
                    if(frame_num >= step_len)
                    {
                        frame_num -= step_len;
                    }
                    else if(frame_num > 0)
                    {
                        frame_num = 0; //����һ֡, ����һ֡
                    }

                    buf_offset = (height - frame_num) * (width / 8); //����ƫ��
                    hal_lcd_fill_picture(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);

                    vTaskDelay( Lcd_Slide_Config.speed_ms / portTICK_PERIOD_MS );
                }
                break;
            case Lcd_Slide_Bottom:
                frame_num = height; //������ˢ�µ�֡��
                while (frame_num)
                {
                    if(frame_num >= step_len)
                    {
                        frame_num -= step_len;
                    }
                    else if(frame_num > 0)
                    {
                        frame_num = 0; //����һ֡, ����һ֡
                    }

                    buf_offset = (height * width / 8) - (height - frame_num) * (width / 8); //����ƫ��
                    hal_lcd_fill_picture(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);

                    vTaskDelay( Lcd_Slide_Config.speed_ms / portTICK_PERIOD_MS );
                }
                break;
            case Lcd_Slide_Left:
                if(step_len%8 == 0 ) //������8�ı���
                {
                    frame_num = width/8; //���֡��
                    buf_offset = 0;

                    while (frame_num)
                    {
                        if(frame_num >= step_len/8)
                        {
                            frame_num -= step_len/8;
                            buf_offset += step_len/8;
                        }
                        else if(frame_num > 0)
                        {
                            buf_offset = width/8;
                            frame_num = 0;
                        }

                        //ˢ��һ������һ��
                        hal_lcd_fill_picture_jump(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);                 
                    }
                }
                else
                {
                    //��������8�ı������漰������λ����, ��������Ҫ�ڴ���Ӻ���
                }
                break;
            case Lcd_Slide_Right:
                if(step_len%8 == 0 ) //������8�ı���
                {
                    frame_num = width/8; //���֡��
                    buf_offset = width/8;
                    while (frame_num)
                    {
                        if(frame_num >= step_len/8)
                        {
                            frame_num -= step_len/8;
                            buf_offset -= step_len/8;
                        }
                        else if(frame_num > 0)
                        {
                            buf_offset = 0;
                            frame_num = 0;
                        }
                        
                        //ˢ��һ������һ��
                        hal_lcd_fill_picture_jump(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);
                    }
                }
                else
                {
                    //��������8�ı������漰������λ����, ��������Ҫ�ڴ���Ӻ���
                }
                break;
            default:
                break;
        }
    }
    else
    {
        //��ɫ��һ��, src �� buf ����Ҫ�ֿ�ˢ��, ���������ˢ��Ч�����ܲ�̫��
    }

    //һ��Ҫ�ͷſռ�
    free(buf); 
    return LCD_DISPLAY_OK;
}


/**
 * @brief ������������(��֪����), ��ȡ����ͼ������ֵ
 * @param weather_code - ��������, ����������鿴 weather_code
 * @return ������֪����ͼ������, ������鿴 Weather_Icon_Index_e
 * @note ��֪��������: https://docs.seniverse.com/api/start/code.html
 */
Weather_Icon_Index_e hal_lcd_get_weather_icon_index(Weather_Code_e weather_code)
{
    Weather_Icon_Index_e icon_index = Weather_Icon_Index_Unknown;

    switch (weather_code)
    {
        case Weather_Code_Sunny:                     //0 - �磨���ڳ��а����磩
            icon_index = Weather_Icon_Index_Sunny;
            break;
        case Weather_Code_Clear:                     //1 - �磨���ڳ���ҹ���磩
            icon_index = Weather_Icon_Index_Clear;
            break;
        case Weather_Code_Fair:                      //2 - �磨������а����磩
            icon_index = Weather_Icon_Index_Fair;
            break;
        case Weather_Code_Fair_Night:                //3 - �磨�������ҹ���磩
            icon_index = Weather_Icon_Index_Fair_Night;
            break;
        case Weather_Code_Cloudy:                    //4 - ����
            icon_index = Weather_Icon_Index_Cloudy;
            break;
        case Weather_Code_Partly_Cloudy:             //5 - ������ - ����
            icon_index = Weather_Icon_Index_Partly_Cloudy;
            break;
        case Weather_Code_Partly_Cloudy_Night:       //6 - ������ - ����
            icon_index = Weather_Icon_Index_Partly_Cloudy_Night;
            break;
        case Weather_Code_Mostly_Cloudy:             //7 - �󲿶��� - ����
            icon_index = Weather_Icon_Index_Mostly_Cloudy;
            break;
        case Weather_Code_Mostly_Cloudy_Night:       //8 - �󲿶��� - ����
            icon_index = Weather_Icon_Index_Mostly_Cloudy_Night;
            break;
        case Weather_Code_Overcast:                  //9 - ��
            icon_index = Weather_Icon_Index_Overcast;
            break;
        case Weather_Code_Shower:                    //10 - ����
            icon_index = Weather_Icon_Index_Shower;
            break;
        case Weather_Code_Thundershower:             //11 - ������
            icon_index = Weather_Icon_Index_Thundershower;
            break;
        case Weather_Code_Thundershower_with_Hail:   //12 - ��������б���
            icon_index = Weather_Icon_Index_Thundershower_with_Hail;
            break;
        case Weather_Code_Light_Rain:                //13 - С��
            icon_index = Weather_Icon_Index_Light_Rain;
            break;
        case Weather_Code_Moderate_Rain:             //14 - ����
            icon_index = Weather_Icon_Index_Moderate_Rain;
            break;
        case Weather_Code_Heavy_Rain:                //15 - ����
            icon_index = Weather_Icon_Index_Heavy_Rain;
            break;
        case Weather_Code_Storm:                     //16 - ����
            icon_index = Weather_Icon_Index_Storm;
            break;
        case Weather_Code_Heavy_Storm:               //17 - ����
            icon_index = Weather_Icon_Index_Heavy_Storm;
            break;
        case Weather_Code_Severe_Storm:              //18 - �ش���
            icon_index = Weather_Icon_Index_Severe_Storm;
            break;
        case Weather_Code_Ice_Rain:                  //19 - ����
            icon_index = Weather_Icon_Index_Ice_Rain;
            break;
        case Weather_Code_Sleet:                     //20 - ���ѩ
            icon_index = Weather_Icon_Index_Sleet;
            break;
        case Weather_Code_Snow_Flurry:               //21 - ��ѩ
            icon_index = Weather_Icon_Index_Snow_Flurry;
            break;
        case Weather_Code_Light_Snow:                //22 - Сѩ
            icon_index = Weather_Icon_Index_Light_Snow;
            break;
        case Weather_Code_Moderate_Snow:             //23 - ��ѩ
            icon_index = Weather_Icon_Index_Moderate_Snow;
            break;
        case Weather_Code_Heavy_Snow:                //24 - ��ѩ
            icon_index = Weather_Icon_Index_Heavy_Snow;
            break;
        case Weather_Code_Snowstorm:                 //25 - ��ѩ
            icon_index = Weather_Icon_Index_Snowstorm;
            break;
        case Weather_Code_Dust:                      //26 - ����
            icon_index = Weather_Icon_Index_Dust;
            break;
        case Weather_Code_Sand:                      //27 - ��ɳ
            icon_index = Weather_Icon_Index_Sand;
            break;
        case Weather_Code_Duststorm:                 //28 - ɳ����
            icon_index = Weather_Icon_Index_Duststorm;
            break;
        case Weather_Code_Sandstorm:                 //29 - ǿɳ����
            icon_index = Weather_Icon_Index_Sandstorm;
            break;
        case Weather_Code_Foggy:                     //30 - ��
            icon_index = Weather_Icon_Index_Foggy;
            break;
        case Weather_Code_Haze:                      //31 - ��
            icon_index = Weather_Icon_Index_Haze;
            break;
        case Weather_Code_Windy:                     //32 - ��
            icon_index = Weather_Icon_Index_Windy;
            break;
        case Weather_Code_Blustery:                  //33 - ���
            icon_index = Weather_Icon_Index_Blustery;
            break;
        case Weather_Code_Hurricane:                 //34 - 쫷�
            icon_index = Weather_Icon_Index_Hurricane;
            break;
        case Weather_Code_Tropical_Storm:            //35 - �ȴ��籩
            icon_index = Weather_Icon_Index_Tropical_Storm;
            break;
        case Weather_Code_Tornado:                   //36 - �����
            icon_index = Weather_Icon_Index_Tornado;
            break;
        case Weather_Code_Cold:                      //37 - ��
            icon_index = Weather_Icon_Index_Cold;
            break;
        case Weather_Code_Hot:                       //38 - ��
            icon_index = Weather_Icon_Index_Hot;
            break;
        case Weather_Code_Unknown:                   //99 - δ֪
            icon_index = Weather_Icon_Index_Unknown;
            break;

        default:
            icon_index = Weather_Icon_Index_Unknown;
            break;
    }

    return icon_index;
}


/**
 * @brief ��ʾ����ͼ��
 * @param x - ������
 * @param y - ������
 * @param weather_code - �������
 * @return ����������� LCD_DISPLAY_OK, �����������ֵ��鿴 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_show_weather_icon(uint16_t x, uint16_t y,  uint16_t weather_code)
{
    Weather_Icon_Index_e icon_index = Weather_Icon_Index_Unknown;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //��ȡͼ������
    icon_index = hal_lcd_get_weather_icon_index(weather_code);

    //�����ж�
    ret = hal_lcd_check_xy_area(x, y, LCD_WEATHER_ICON_WIDTH, LCD_WEATHER_ICON_HEIGHT);
    if (ret != LCD_DISPLAY_OK)
    {
        return ret;
    }

    if( icon_index != Weather_Icon_Index_Unknown )
    {
        hal_lcd_fill_picture(x, y, LCD_WEATHER_ICON_WIDTH, LCD_WEATHER_ICON_HEIGHT, LCD_WEATHER_ICON_COLOR, LCD_WEATHER_ICON_BACK_COLOR, weather_icon_tab[icon_index]);
    }
        
    return LCD_DISPLAY_OK;
}


/**
 * @brief �����л�������
 * @param date - ʱ����Ϣ, ������鿴 Date_t, ����Ϣ�� page 0 ��ʾ
 * @param weather_code - ��������, ������鿴 Weather_Code_e, ����Ϣ�� page 1 ��ʾ
 * @param temperature - �¶�, ����Ϣ�� page 1 ��ʾ
 * @param main_page_num - ��һҳ����ҳ��(��ǰ������ʾ�Ļ���), �ò���ֻ����0 ���� 1, 
 * @return none
 */
void hal_lcd_main_page_slide(Date_t date, Weather_Code_e weather_code, int temperature, uint8_t main_page_num)
{
    uint16_t slide_width, slide_height;   //�õ�Ƭ�����С
    uint16_t x, y; //�õ�Ƭ������ʼ��ַ
    uint16_t icon_index = 0;

    uint8_t str_date_buf[30] = {0};
    uint8_t str_time_buf[30] = {0};
    uint8_t str_temperature_buf[30] = {0};

    uint8_t page0_buf[240*80/8] = {0};  //��0ҳ
    uint8_t page1_buf[240*80/8] = {0};  //��1ҳ
    uint16_t buf_offset_1 = 0, buf_offset_2 = 0, buf_offset_3=0, buf_offset_4=0, buf_offset_5=0, buf_offset_6=0;

    uint8_t du[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,
    0x08,0x10,0x08,0x10,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* �� */
    };

    x = 0;
    y = 55;

    if(main_page_num > 1)
        return;

    //�õ�Ƭ��ʼ��ַ
    x = LCD_DATE_SHOW_X;
    y = LCD_DATE_SHOW_Y;

    //�õ�Ƭ��͸�
    slide_width = LCD_WIDTH;
    slide_height = 80; //����߶���Ҫȷ��ȫ�������ܹ�������ʾ

    //�����ݸ��Ƶ�page0
    //�������� - ���� 2022-08-14 ����    23:09:25
    sprintf((char *)str_date_buf, "%4d-%02d-%02d     ", date.year, date.month, date.day);
    sprintf((char *)str_time_buf, "%02d:%02d:%02d", date.hour, date.minute, date.second);
    buf_offset_4 = 48*30 + 7;

    buf_offset_5 = 27*30 + 15;
    if(temperature > -100 && temperature < -9)
    {
        //ռ��3���ֽ�
        sprintf((char *)str_temperature_buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)str_temperature_buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //�¶�ռ��1���ֽ�
        sprintf((char *)str_temperature_buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //�¶�ռ��2���ֽ�
        sprintf((char *)str_temperature_buf, " %d", temperature);
    }

    for (int i = 0; i < 32; i++)
    {
        buf_offset_1 = 30*i;
        buf_offset_2 = 2*i;
        buf_offset_3 = 4*i;
        //���и��� - ����: 2022-08-14 ����
        memcpy( page0_buf + buf_offset_1 + 0,  ascii_3216 + (str_date_buf[0] - 32)*64 + buf_offset_2, 2); //2
        memcpy( page0_buf + buf_offset_1 + 2,  ascii_3216 + (str_date_buf[1] - 32)*64 + buf_offset_2, 2); //0
        memcpy( page0_buf + buf_offset_1 + 4,  ascii_3216 + (str_date_buf[2] - 32)*64 + buf_offset_2, 2); //2
        memcpy( page0_buf + buf_offset_1 + 6,  ascii_3216 + (str_date_buf[3] - 32)*64 + buf_offset_2, 2); //2
        memcpy( page0_buf + buf_offset_1 + 8,  ascii_3216 + (str_date_buf[4] - 32)*64 + buf_offset_2, 2); //-
        memcpy( page0_buf + buf_offset_1 + 10, ascii_3216 + (str_date_buf[5] - 32)*64 + buf_offset_2, 2); //0
        memcpy( page0_buf + buf_offset_1 + 12, ascii_3216 + (str_date_buf[6] - 32)*64 + buf_offset_2, 2); //8
        memcpy( page0_buf + buf_offset_1 + 14, ascii_3216 + (str_date_buf[7] - 32)*64 + buf_offset_2, 2); //-
        memcpy( page0_buf + buf_offset_1 + 16, ascii_3216 + (str_date_buf[8] - 32)*64 + buf_offset_2, 2); //1
        memcpy( page0_buf + buf_offset_1 + 18, ascii_3216 + (str_date_buf[9] - 32)*64 + buf_offset_2, 2); //4
        memcpy( page0_buf + buf_offset_1 + 20, ascii_3216 + (str_date_buf[10] - 32)*64 + buf_offset_2, 2);//�ո�
        memcpy( page0_buf + buf_offset_1 + 22, Chinese_Gbk[2].Msk + buf_offset_3, 4);             //��
        memcpy( page0_buf + buf_offset_1 + 26, Chinese_Gbk[date.week + 3].Msk + buf_offset_3, 4); //��

        // vTaskDelay(1);

        //���и��� - ����: 23:09:25
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 0,  ascii_3216 + (str_time_buf[0] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 2,  ascii_3216 + (str_time_buf[1] - 32)*64 + buf_offset_2, 2); //3
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 4,  ascii_3216 + (str_time_buf[2] - 32)*64 + buf_offset_2, 2); //:
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 6,  ascii_3216 + (str_time_buf[3] - 32)*64 + buf_offset_2, 2); //0
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 8,  ascii_3216 + (str_time_buf[4] - 32)*64 + buf_offset_2, 2); //9
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 10, ascii_3216 + (str_time_buf[5] - 32)*64 + buf_offset_2, 2); //:
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 12, ascii_3216 + (str_time_buf[6] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 14, ascii_3216 + (str_time_buf[7] - 32)*64 + buf_offset_2, 2); //5

        //���и����¶� - ����: 32��
        //�¶ȹ�ռ3���ֽ�, ���ϵ�λ ��
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 0, ascii_3216 + (str_temperature_buf[0] - 32)*64 + buf_offset_2, 2); //�ո�
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 2, ascii_3216 + (str_temperature_buf[1] - 32)*64 + buf_offset_2, 2); //3
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 4, ascii_3216 + (str_temperature_buf[2] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 8, du + buf_offset_2, 2); //��
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 10, ascii_3216 + ('C' - 32)*64 + buf_offset_2, 2); //C
    }

    icon_index = hal_lcd_get_weather_icon_index(weather_code);
    buf_offset_6 = 15*30 + 7;
    for (int i = 0; i < 56; i++)
    {
        //���и�������ͼ��
        buf_offset_1 = 30*i;
        memcpy(page1_buf + buf_offset_6 + buf_offset_1, &weather_icon_tab[icon_index][i*7], 7);
    }
    
    
    //��֤�����Ƿ���ȷ
    //hal_lcd_fill_picture(x, y, slide_width, slide_height, COLOR_WHITE, COLOR_BLACK, page0_buf);
    //hal_lcd_fill_picture(x, y, slide_width, slide_height, COLOR_WHITE, COLOR_BLACK, page1_buf);


    Lcd_Slide_Config_t Lcd_Slide_Config;

    //�õ�Ƭ����
    Lcd_Slide_Config.x = x;
    Lcd_Slide_Config.y = y;
    Lcd_Slide_Config.width = slide_width;
    Lcd_Slide_Config.height = slide_height;

    //��������
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Top;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Bottom;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Left;
    Lcd_Slide_Config.slide_dir = Lcd_Slide_Right;

    //ͼƬ��ɫ �� ͼƬ
    Lcd_Slide_Config.src_color = COLOR_WHITE;
    Lcd_Slide_Config.src_back_color = COLOR_BLUE;
    if(main_page_num == 0)
    {
        Lcd_Slide_Config.src_buf = (uint8_t *)&page0_buf[0];
    }
    else
    {
        Lcd_Slide_Config.src_buf = (uint8_t *)&page1_buf[0];
    }
    
    Lcd_Slide_Config.dest_color = COLOR_WHITE;
    Lcd_Slide_Config.dest_back_color = COLOR_BLUE;
    if(main_page_num == 0)
    {
        Lcd_Slide_Config.dest_buf = (uint8_t *)&page1_buf[0];
    }
    else
    {
        Lcd_Slide_Config.dest_buf = (uint8_t *)&page0_buf[0];
    }

    //һ֡������ �� һ֡�ƶ��������ص�
    Lcd_Slide_Config.speed_ms = 1;
    Lcd_Slide_Config.step_len = 8;

    hal_lcd_slide_show(Lcd_Slide_Config);
}

/**
 * @brief ������, ���ڿ�ɾ��
 * @param none
 * @return none
 */
void hal_lcd_test(void)
{
#if 0
    //��ҳ�����
    hal_lcd_show_Date(LCD_DATE_SHOW_X, LCD_DATE_SHOW_Y, 2022, 8, 14, Sunday);
    hal_lcd_show_time(LCD_TIME_SHOW_X, LCD_TIME_SHOW_Y, 23, 9, 25);

    hal_lcd_draw_line(LCD_LINE_X, LCD_LINE_Y, LCD_WIDTH, LCD_LINE_HEIGHT, COLOR_WHITE);

    hal_lcd_show_Small_temperature(LCD_TEMP_SMALL_SHOW_X, LCD_TEMP_SMALL_SHOW_Y, 32);
    hal_lcd_show_small_time(LCD_SMALL_TIME_SHOW_X, LCD_SMALL_TIME_SHOW_Y, 23, 9);
#endif

#if 0
    //��ҳ�����
    hal_lcd_show_weather_icon(LCD_WEATHER_ICON_X, LCD_WEATHER_ICON_Y, Weather_Code_Mostly_Cloudy);
    hal_lcd_show_temperature(LCD_TEMP_SHOW_X, LCD_TEMP_SHOW_Y, 32);

    hal_lcd_draw_line(LCD_LINE_X, LCD_LINE_Y, LCD_WIDTH, LCD_LINE_HEIGHT, LCD_LINE_COLOR);

    hal_lcd_show_Small_temperature(LCD_TEMP_SMALL_SHOW_X, LCD_TEMP_SMALL_SHOW_Y, 32);
    hal_lcd_show_small_time(LCD_SMALL_TIME_SHOW_X, LCD_SMALL_TIME_SHOW_Y, 9, 30);
#endif

#if 0
    //�õ�Ƭ����
    Lcd_Slide_Config_t Lcd_Slide_Config;

    //�õ�Ƭ����
    Lcd_Slide_Config.x = 50;
    Lcd_Slide_Config.y = 50;
    Lcd_Slide_Config.width = 56;
    Lcd_Slide_Config.height = 56;

    //��������
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Top;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Bottom;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Left;
    Lcd_Slide_Config.slide_dir = Lcd_Slide_Right;

    //ͼƬ��ɫ �� ͼƬ
    Lcd_Slide_Config.src_color = COLOR_WHITE;
    Lcd_Slide_Config.src_back_color = COLOR_BLUE;
    Lcd_Slide_Config.src_buf = (uint8_t *)weather_icon_tab[0];

    Lcd_Slide_Config.dest_color = COLOR_WHITE;
    Lcd_Slide_Config.dest_back_color = COLOR_BLUE;
    Lcd_Slide_Config.dest_buf = (uint8_t *)weather_icon_tab[1];

    //һ֡������ �� һ֡�ƶ��������ص�
    Lcd_Slide_Config.speed_ms = 1;
    Lcd_Slide_Config.step_len = 8;

    printf("%d\n", hal_lcd_slide_show(Lcd_Slide_Config));
#endif

#if 1
    //����������õ�Ƭ
    Date_t date = {
        .year = 2022,
        .month = 8,
        .day = 14,

        .hour = 23,
        .minute = 9,
        .second = 25,
        .week = Sunday,
    };
    Weather_Code_e weather_code = Weather_Code_Sunny;
    hal_lcd_main_page_slide(date, weather_code, 32, 0);

    hal_lcd_draw_line(LCD_LINE_X, LCD_LINE_Y, LCD_WIDTH, LCD_LINE_HEIGHT, LCD_LINE_COLOR);

    hal_lcd_show_Small_temperature(LCD_TEMP_SMALL_SHOW_X, LCD_TEMP_SMALL_SHOW_Y, 32);
    hal_lcd_show_small_time(LCD_SMALL_TIME_SHOW_X, LCD_SMALL_TIME_SHOW_Y, 9, 30);
#endif

}

