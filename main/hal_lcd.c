#include "hal_lcd.h"

/* 此文件实现屏幕功能函函数 */

/**
 * @brief 检查区域是否合规
 * @param start_x - x轴
 * @param start_y - y轴
 * @param width - 宽
 * @param height - 高
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_check_xy_area(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height)
{
    /* 判断该坐标是否能显示一个字符 */
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //坐标错误, 直接退出
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    if( (start_x > (LCD_WIDTH - (width-1)) ) || (start_y > (LCD_HEIGHT - (height-1))))
    {
        //溢出屏幕
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if(width == 0 || height == 0)
    {
        //区域问题
        return LCD_DISPLAY_AREA_ERR;
    }

    return LCD_DISPLAY_OK;
}




/**
 * @brief 显示单个汉字
 * @param start_x - x轴
 * @param start_y - y轴
 * @param font_color - 字体颜色
 * @param back_color - 背景颜色
 * @param p_str - 汉字字符, 注意使用编码格式是gb2312, 汉字占2个字节
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_chinese(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_str)
{
    int i=0, font_num=0;
    uint8_t f_buf[2], b_buf[2];
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //默认能找到汉字

    //将16bit颜色数据转换为数组, 方便后面使用
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //位置判断
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //地址错误, 直接退出
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* 判断该坐标是否能显示完整的汉字 */
    if( (start_x > LCD_WIDTH - FONT_CHINESE_WIDTH) || (start_y > LCD_HEIGHT - FONT_CHINESE_HEIGHT))
    {
        //坐标错误, 直接退出
        return LCD_DISPLAY_OVERFLOW_ERR;
    }
    else
    {
        if(p_str[0] < 128)
        {
            //不是中文汉字
            ret = LCD_DISPLAY_NOT_FOUND_ERR;
        }
        else
        {
            //获取字库汉字总数
            //font_num = sizeof(Chinese_Gbk) / sizeof(typFNT_GB2312_t); //sizeof无法使用, 使用宏定义获取字库数量
            font_num = CHINESE_GBK_NUM; //数组长度
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
                //没找到到汉字
                ret = LCD_DISPLAY_NOT_FOUND_ERR;
                printf("chinese buf not found chinese array\n");
            }
            else
            {
                //找到汉字, 设置坐标
                lcd_address_window_set(start_x, start_y, start_x + FONT_CHINESE_WIDTH-1, start_y + FONT_CHINESE_HEIGHT-1);

                //显示汉字
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
 * @brief 显示单个字符(32*16)
 * @param start_x - x轴
 * @param start_y - y轴
 * @param font_color - 字体颜色
 * @param back_color - 背景颜色
 * @param p_char - 英文字符
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_char(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_char)
{
    uint8_t f_buf[2], b_buf[2], tmp=0;
    int index=0, char_array_size=0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //默认正常

    //将16bit颜色数据转换为数组, 方便后面使用
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //位置判断
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //地址错误, 直接退出
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* 判断该坐标是否能显示一个字符 */
    if( (start_x > LCD_WIDTH - FONT_STRING_WIDTH) || (start_y > LCD_HEIGHT - FONT_STRING_HEIGHT))
    {
        //坐标错误, 直接退出
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if((*p_char > 128) || (*p_char<32))
    {
        //不是英文字符
        ret = LCD_DISPLAY_NOT_FOUND_ERR;
    }
    else
    {
        //获取单个字符, 数组大小
        char_array_size = FONT_STRING_WIDTH/8*FONT_STRING_HEIGHT;

        //获取起始地址
        index = (*p_char - 32) * char_array_size;

        //设置坐标
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
 * @brief 显示单个字符(16*88)
 * @param start_x - x轴
 * @param start_y - y轴
 * @param font_color - 字体颜色
 * @param back_color - 背景颜色
 * @param p_char - 英文字符
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
static Lcd_Diplay_State_e hal_lcd_draw_char_1608(uint16_t start_x, uint16_t start_y, Lcd_Color_e font_color, Lcd_Color_e back_color, uint8_t *p_char)
{
    uint8_t f_buf[2], b_buf[2], tmp=0;
    int index=0, char_array_size=0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK; //默认正常

    //将16bit颜色数据转换为数组, 方便后面使用
    f_buf[0] = (uint8_t)(font_color>>8);
    f_buf[1] = (uint8_t)(font_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    //位置判断
    if(start_x > LCD_WIDTH-1 || start_y > LCD_HEIGHT-1)
    {
        //地址错误, 直接退出
        return LCD_DISPLAY_ADDRESS_ERR;
    }

    /* 判断该坐标是否能显示一个字符 */
    if( (start_x > LCD_WIDTH - 8 +1) || (start_y > LCD_HEIGHT - 16+1))
    {
        //坐标错误, 直接退出
        return LCD_DISPLAY_OVERFLOW_ERR;
    }

    if((*p_char > 128) || (*p_char<32))
    {
        //不是英文字符
        ret = LCD_DISPLAY_NOT_FOUND_ERR;
    }
    else
    {
        //获取单个字符, 数组大小
        char_array_size = 16;

        //获取起始地址
        index = (*p_char - 32) * char_array_size;

        //设置坐标
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
 * @brief 显示16*8字符串
 * @param start_x - x轴
 * @param start_y - y轴
 * @param font_color - 字体颜色
 * @param back_color - 背景颜色
 * @param p_str - 字符串
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
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
            //换行
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
                //字符显示成功
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
 * @brief 显示中英文字符串
 * @param start_x - x轴
 * @param start_y - y轴
 * @param font_color - 字体颜色
 * @param back_color - 背景颜色
 * @param p_str - 字符串
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 * @note 指定位置显示中英文字符。 中文32*32, 英文16*32, 中文字符采用gb2312编码
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
            //换行
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
                //字符显示成功
                x += FONT_STRING_WIDTH;
                tmp += 1;
                break;

            case LCD_DISPLAY_NOT_FOUND_ERR:
                //没找到字符，尝试找中文字符
                ret = hal_lcd_draw_chinese(x, y, font_color, back_color, tmp);

                switch(ret)
                {
                    case LCD_DISPLAY_OK:
                    case LCD_DISPLAY_NOT_FOUND_ERR:
                        x += FONT_CHINESE_WIDTH;
                        tmp += 2;
                        break;

                    case LCD_DISPLAY_ADDRESS_ERR:  //地址错误
                    case LCD_DISPLAY_OVERFLOW_ERR: //溢出屏幕
                        //如果地址出错, 直接退出
                        printf("chinese addr err\n");
                        return LCD_DISPLAY_ADDRESS_ERR;

                    default:
                        break;
                }
                break;

            case LCD_DISPLAY_ADDRESS_ERR:  //地址错误
            case LCD_DISPLAY_OVERFLOW_ERR: //溢出屏幕
                //如果地址出错, 直接退出
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
 * @brief 指定区域显示图片
 * @param start_x - x轴
 * @param start_y - y轴
 * @param width - 填充区域宽
 * @param height - 填充区域高
 * @param fill_color - 填充图案的颜色
 * @param back_color - 背景颜色
 * @param p_buf - 数组起始地址
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_fill_picture(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Lcd_Color_e fill_color, Lcd_Color_e back_color, uint8_t *p_buf)
{
    uint8_t f_buf[2], b_buf[2];
    uint16_t buf_size = 0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //将16bit颜色数据转换为数组, 方便后面使用
    f_buf[0] = (uint8_t)(fill_color>>8);
    f_buf[1] = (uint8_t)(fill_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //返回错误信息
        return ret;
    }

    //设置坐标
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    buf_size = width/8*height;

    /* 刷新图案 */
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
 * @brief 画一条线
 * @param start_x - x轴
 * @param start_y - y轴
 * @param width - 线段宽
 * @param height - 线段高
 * @param line_color - 线段颜色
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_draw_line(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t  height, Lcd_Color_e line_color)
{
    uint8_t line_buf[2];
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //将16bit颜色数据转换为数组, 方便后面使用
    line_buf[0] = (uint8_t)(line_color>>8);
    line_buf[1] = (uint8_t)(line_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //返回错误信息
        return ret;
    }

    //设置坐标
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    for (int i = 0; i < width*height; i++)
    {
        st7735_data(line_buf, 2);
    }

    return LCD_DISPLAY_OK;
}


/**
 * @brief 显示日期和星期
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param year - 年
 * @param month - 月
 * @param day - 日
 * @param week - 星期, 具体请查看 Week_e
 * @return none
 */
void hal_lcd_show_Date(uint16_t x, uint16_t y, uint16_t year, uint16_t month, uint16_t day, Week_e week)
{
    uint8_t str_buf[30]; //存储日期字符串

    //x = LCD_DATE_SHOW_X;
    //y = LCD_DATE_SHOW_Y;

    //处理日期 - 占用11个字节, 剩下星期部分初始化为空格
    sprintf((char *)str_buf, "%4d-%02d-%02d     ", year, month, day);

    //处理星期 -- 占用2个字节
    switch(week)
    {
        case Monday:
            sprintf((char *)&str_buf[11], "周%s", "一");
            break;
        case Tuesday:
            sprintf((char *)&str_buf[11], "周%s", "二");
            break;
        case Wednesday:
            sprintf((char *)&str_buf[11], "周%s", "三");
            break;
        case Thursday:
            sprintf((char *)&str_buf[11], "周%s", "四");
            break;
        case Friday:
            sprintf((char *)&str_buf[11], "周%s", "五");
            break;
        case Saturday:
            sprintf((char *)&str_buf[11], "周%s", "六");
            break;
        case Sunday:
            sprintf((char *)&str_buf[11], "周%s", "日");
            break;
        default:
            break;
    }

    hal_lcd_draw_font(x, y, LCD_DATE_SHOW_COLOR, LCD_DATE_SHOW_BACK_COLOR, str_buf);
}


/**
 * @brief 显示时间
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param hour - 时
 * @param min - 分
 * @param sec - 秒
 * @return none
 */
void hal_lcd_show_time(uint16_t x, uint16_t y, uint8_t hour, uint8_t min, uint8_t sec)
{
    uint8_t str_buf[30];

    //设置显示位置
    // x = LCD_TIME_SHOW_X;
    // y = LCD_TIME_SHOW_Y;

    sprintf((char *)&str_buf[0], "%02d:%02d:%02d", hour, min, sec);

    //显示
    hal_lcd_draw_font(x, y, LCD_TIME_SHOW_COLOR, LCD_TIME_SHOW_BACK_COLOR, str_buf);
}

/**
 * @brief 显示时间(小字体)
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param hour - 时
 * @param min - 分
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
 * @brief 显示温度，正常字体
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param temperature - 温度
 * @note 1.显示范围 -99℃ - 99℃, 并且只显示整数
 */
void hal_lcd_show_temperature(uint16_t x, uint16_t y, int temperature)
{
    uint8_t buf[30] = {0};
    uint8_t du[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,
    0x08,0x10,0x08,0x10,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* ° */
    };

    if(temperature < -99 || temperature > 99)
        return;

    if(temperature > -100 && temperature < -9)
    {
        //占用3个字节
        sprintf((char *)buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //温度占用2个字节
        sprintf((char *)buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //温度占用1个字节
        sprintf((char *)buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //温度占用2个字节
        sprintf((char *)buf, " %d", temperature);
    }
    
    //刷新温度
    hal_lcd_draw_font(x, y, COLOR_WHITE, COLOR_BLACK, (uint8_t *)buf);
    hal_lcd_fill_picture(x + 48, y, 16, 32, COLOR_WHITE, COLOR_BLACK, du);
    hal_lcd_draw_font(x + 64, y, COLOR_WHITE, COLOR_BLACK, (uint8_t *)"C");
}


/**
 * @brief 显示温度，字体大小16*8
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param temperature - 温度
 * @note 1.显示范围 -99℃ - 99℃, 并且只显示整数
 */
void hal_lcd_show_Small_temperature(uint16_t x, uint16_t y, int temperature)
{
    uint8_t buf[30] = {0};
    uint8_t du[] = {0x00,0x00,0x18,0x24,0x24,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"°",0*/};

    // x = LCD_TEMP_SMALL_SHOW_X;
    // y = LCD_TEMP_SMALL_SHOW_Y;

    if(temperature < -99 || temperature > 99)
        return;

    if(temperature > -100 && temperature < -9)
    {
        //占用3个字节
        sprintf((char *)buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //温度占用2个字节
        sprintf((char *)buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //温度占用1个字节
        sprintf((char *)buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //温度占用2个字节
        sprintf((char *)buf, " %d", temperature);
    }

    //刷新温度
    hal_lcd_draw_string_1608(x, y, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, (uint8_t *)buf);
    hal_lcd_fill_picture(x + 24, y, 8, 16, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, du);
    hal_lcd_draw_string_1608(x + 32, y, LCD_TEMP_SMALL_SHOW_COLOR, LCD_TEMP_SMALL_SHOW_BACK_COLOR, (uint8_t *)"C");
}


/**
 * @brief 指定区域显示图片, 刷新一行, 跳过一行, 此函数目前只在 hal_lcd_slide_show 函数中用到
 * @param start_x - x轴
 * @param start_y - y轴
 * @param width - 填充区域宽
 * @param height - 填充区域高
 * @param fill_color - 填充图案的颜色
 * @param back_color - 背景颜色
 * @param p_buf - 数组起始地址
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e, 
 */
Lcd_Diplay_State_e hal_lcd_fill_picture_jump(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, Lcd_Color_e fill_color, Lcd_Color_e back_color, uint8_t *p_buf)
{
    uint8_t f_buf[2], b_buf[2];
    uint16_t buf_size = 0;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;
    uint8_t *tmp = p_buf;

    //将16bit颜色数据转换为数组, 方便后面使用
    f_buf[0] = (uint8_t)(fill_color>>8);
    f_buf[1] = (uint8_t)(fill_color);

    b_buf[0] = (uint8_t)(back_color>>8);
    b_buf[1] = (uint8_t)(back_color);

    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if(ret != LCD_DISPLAY_OK)
    {
        //返回错误信息
        return ret;
    }

    //设置坐标
    lcd_address_window_set(start_x, start_y, start_x+width-1, start_y+height-1);

    buf_size = width/8*height;

    //一行一行刷新
    for (int i = 0; i < buf_size; i++)
    {
        //每一行数组的起始位置
        if(i % (width/8) == 0 && i/(width/8) != 0)
        {
            tmp += width/8;
        }

        //刷新一行数据
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
 * @brief 幻灯片(单色)
 * @param Lcd_Slide_Config - 具体配置请查看 Lcd_Slide_Config_t
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 * @note 1.此函数中有循环延时，建议创建任务调用此函数
 *       2.幻灯片区域的宽需要是8的倍数
 *       3.如果刷新方向是左或者右, 那么步长必须是8的倍数, 因为任意步长实现比较麻烦, 我不想写
 *       4.目前只能刷新原图和目标图, 颜色一致的图片, 不同颜色刷新我也不想写
 */
Lcd_Diplay_State_e hal_lcd_slide_show(Lcd_Slide_Config_t Lcd_Slide_Config)
{
    uint16_t frame_num=0; //帧数
    uint16_t start_x, start_y, width, height; //幻灯片区域
    uint16_t src_color, src_back, dest_color, dest_back; //颜色
    uint16_t step_len; //步长
    uint8_t  *buf; 
    uint8_t  color_same_flag = 0; 

    uint16_t buf_offset = 0; //buf偏移部分

    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    start_x = Lcd_Slide_Config.x;
    start_y = Lcd_Slide_Config.y;
    width = Lcd_Slide_Config.width;
    height = Lcd_Slide_Config.height;

    src_color = Lcd_Slide_Config.src_color;
    src_back = Lcd_Slide_Config.src_back_color;
    dest_color = Lcd_Slide_Config.dest_color;
    dest_back = Lcd_Slide_Config.dest_back_color;

    //步长
    step_len = Lcd_Slide_Config.step_len;

    //步长判断
    if(step_len == 0)
    {
        //步长错误
        return LCD_DISPLAY_SLIDE_STEP_ERR;
    }
    if(Lcd_Slide_Config.slide_dir == Lcd_Slide_Left || Lcd_Slide_Config.slide_dir == Lcd_Slide_Right)
    {
        if( step_len%8 != 0)
            return LCD_DISPLAY_SLIDE_STEP_ERR;
    }

    //区域判断
    ret = hal_lcd_check_xy_area(start_x, start_y, width, height);
    if (ret != LCD_DISPLAY_OK)
    {
        return ret;
    }

    //申请原图2倍空间
    buf = (uint8_t *)malloc( width/8*height * 2 );

    //判断原图和目标图片颜色 和 背景色是否一致
    if(src_color == dest_color && src_back == dest_back)
    {
        color_same_flag = 1;
    }

    switch(Lcd_Slide_Config.slide_dir)
    {
        case Lcd_Slide_Top:
            //向上移动, src_buf 后面接着 dest_buf 即可
            //复制 src_buf 到 buf 中
            memcpy(buf, Lcd_Slide_Config.src_buf, width/8 * height);

            //复制 dest_buf 到 buf 中
            memcpy(buf + width/8*height, Lcd_Slide_Config.dest_buf, width/8 * height);
            break;
        case Lcd_Slide_Bottom:
            //向上移动, dest_buf 后面接着 src_buf 即可
            //复制 dest_buf 到 buf 中
            memcpy(buf + width/8*height, Lcd_Slide_Config.src_buf, width/8 * height);

            //复制 src_buf 到 buf 中
            memcpy(buf, Lcd_Slide_Config.dest_buf, width/8 * height);
            break;
        case Lcd_Slide_Left:
            //向左移动, src_buf 和 dest_buf 按行交替存放
            for (int i = 0; i < height; i++)
            {
                //复制 src_buf
                memcpy(buf + 2*i*width/8, Lcd_Slide_Config.src_buf + i*width/8, width/8);

                //复制 dest_buf
                memcpy(buf + (2*i+1)*width/8 , Lcd_Slide_Config.dest_buf + i*width/8, width/8);
            }

            break;
        case Lcd_Slide_Right:
            //向右移动, dest_buf 和 src_buf 按行交替存放
            for (int i = 0; i < height; i++)
            {
                //复制 dest_buf 
                memcpy(buf + 2*i*width/8, Lcd_Slide_Config.dest_buf + i*width/8, width/8);

                //复制 src_buf
                memcpy(buf + (2*i+1)*width/8 , Lcd_Slide_Config.src_buf + i*width/8, width/8);
            }
            break;
        default:
            return LCD_DISPLAY_SLIDE_DIR_ERR; //返回方向错误
    }
    
    if(color_same_flag)
    {
        //如果颜色一致, src 和 buf 不用分开刷图
        switch (Lcd_Slide_Config.slide_dir)
        {
            case Lcd_Slide_Top:
                frame_num = height; //最多可以刷新的帧数
                while (frame_num)
                {
                    if(frame_num >= step_len)
                    {
                        frame_num -= step_len;
                    }
                    else if(frame_num > 0)
                    {
                        frame_num = 0; //不足一帧, 当做一帧
                    }

                    buf_offset = (height - frame_num) * (width / 8); //向下偏移
                    hal_lcd_fill_picture(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);

                    vTaskDelay( Lcd_Slide_Config.speed_ms / portTICK_PERIOD_MS );
                }
                break;
            case Lcd_Slide_Bottom:
                frame_num = height; //最多可以刷新的帧数
                while (frame_num)
                {
                    if(frame_num >= step_len)
                    {
                        frame_num -= step_len;
                    }
                    else if(frame_num > 0)
                    {
                        frame_num = 0; //不足一帧, 当做一帧
                    }

                    buf_offset = (height * width / 8) - (height - frame_num) * (width / 8); //向上偏移
                    hal_lcd_fill_picture(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);

                    vTaskDelay( Lcd_Slide_Config.speed_ms / portTICK_PERIOD_MS );
                }
                break;
            case Lcd_Slide_Left:
                if(step_len%8 == 0 ) //步长是8的倍数
                {
                    frame_num = width/8; //最大帧数
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

                        //刷新一行跳过一行
                        hal_lcd_fill_picture_jump(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);                 
                    }
                }
                else
                {
                    //步长不是8的倍数，涉及数组移位操作, 后期有需要在此添加函数
                }
                break;
            case Lcd_Slide_Right:
                if(step_len%8 == 0 ) //步长是8的倍数
                {
                    frame_num = width/8; //最大帧数
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
                        
                        //刷新一行跳过一行
                        hal_lcd_fill_picture_jump(start_x, start_y, width, height, src_color, src_back, buf + buf_offset);
                    }
                }
                else
                {
                    //步长不是8的倍数，涉及数组移位操作, 后期有需要在此添加函数
                }
                break;
            default:
                break;
        }
    }
    else
    {
        //颜色不一致, src 和 buf 部分要分开刷新, 这种情况下刷新效果可能不太好
    }

    //一定要释放空间
    free(buf); 
    return LCD_DISPLAY_OK;
}


/**
 * @brief 根据天气代码(心知天气), 获取天气图标索引值
 * @param weather_code - 天气代码, 天气代码请查看 weather_code
 * @return 返回心知天气图标索引, 详情请查看 Weather_Icon_Index_e
 * @note 心知天气代码: https://docs.seniverse.com/api/start/code.html
 */
Weather_Icon_Index_e hal_lcd_get_weather_icon_index(Weather_Code_e weather_code)
{
    Weather_Icon_Index_e icon_index = Weather_Icon_Index_Unknown;

    switch (weather_code)
    {
        case Weather_Code_Sunny:                     //0 - 晴（国内城市白天晴）
            icon_index = Weather_Icon_Index_Sunny;
            break;
        case Weather_Code_Clear:                     //1 - 晴（国内城市夜晚晴）
            icon_index = Weather_Icon_Index_Clear;
            break;
        case Weather_Code_Fair:                      //2 - 晴（国外城市白天晴）
            icon_index = Weather_Icon_Index_Fair;
            break;
        case Weather_Code_Fair_Night:                //3 - 晴（国外城市夜晚晴）
            icon_index = Weather_Icon_Index_Fair_Night;
            break;
        case Weather_Code_Cloudy:                    //4 - 多云
            icon_index = Weather_Icon_Index_Cloudy;
            break;
        case Weather_Code_Partly_Cloudy:             //5 - 晴间多云 - 白天
            icon_index = Weather_Icon_Index_Partly_Cloudy;
            break;
        case Weather_Code_Partly_Cloudy_Night:       //6 - 晴间多云 - 晚上
            icon_index = Weather_Icon_Index_Partly_Cloudy_Night;
            break;
        case Weather_Code_Mostly_Cloudy:             //7 - 大部多云 - 白天
            icon_index = Weather_Icon_Index_Mostly_Cloudy;
            break;
        case Weather_Code_Mostly_Cloudy_Night:       //8 - 大部多云 - 晚上
            icon_index = Weather_Icon_Index_Mostly_Cloudy_Night;
            break;
        case Weather_Code_Overcast:                  //9 - 阴
            icon_index = Weather_Icon_Index_Overcast;
            break;
        case Weather_Code_Shower:                    //10 - 阵雨
            icon_index = Weather_Icon_Index_Shower;
            break;
        case Weather_Code_Thundershower:             //11 - 雷阵雨
            icon_index = Weather_Icon_Index_Thundershower;
            break;
        case Weather_Code_Thundershower_with_Hail:   //12 - 雷阵雨伴有冰雹
            icon_index = Weather_Icon_Index_Thundershower_with_Hail;
            break;
        case Weather_Code_Light_Rain:                //13 - 小雨
            icon_index = Weather_Icon_Index_Light_Rain;
            break;
        case Weather_Code_Moderate_Rain:             //14 - 中雨
            icon_index = Weather_Icon_Index_Moderate_Rain;
            break;
        case Weather_Code_Heavy_Rain:                //15 - 大雨
            icon_index = Weather_Icon_Index_Heavy_Rain;
            break;
        case Weather_Code_Storm:                     //16 - 暴雨
            icon_index = Weather_Icon_Index_Storm;
            break;
        case Weather_Code_Heavy_Storm:               //17 - 大暴雨
            icon_index = Weather_Icon_Index_Heavy_Storm;
            break;
        case Weather_Code_Severe_Storm:              //18 - 特大暴雨
            icon_index = Weather_Icon_Index_Severe_Storm;
            break;
        case Weather_Code_Ice_Rain:                  //19 - 冻雨
            icon_index = Weather_Icon_Index_Ice_Rain;
            break;
        case Weather_Code_Sleet:                     //20 - 雨夹雪
            icon_index = Weather_Icon_Index_Sleet;
            break;
        case Weather_Code_Snow_Flurry:               //21 - 阵雪
            icon_index = Weather_Icon_Index_Snow_Flurry;
            break;
        case Weather_Code_Light_Snow:                //22 - 小雪
            icon_index = Weather_Icon_Index_Light_Snow;
            break;
        case Weather_Code_Moderate_Snow:             //23 - 中雪
            icon_index = Weather_Icon_Index_Moderate_Snow;
            break;
        case Weather_Code_Heavy_Snow:                //24 - 大雪
            icon_index = Weather_Icon_Index_Heavy_Snow;
            break;
        case Weather_Code_Snowstorm:                 //25 - 暴雪
            icon_index = Weather_Icon_Index_Snowstorm;
            break;
        case Weather_Code_Dust:                      //26 - 浮尘
            icon_index = Weather_Icon_Index_Dust;
            break;
        case Weather_Code_Sand:                      //27 - 扬沙
            icon_index = Weather_Icon_Index_Sand;
            break;
        case Weather_Code_Duststorm:                 //28 - 沙尘暴
            icon_index = Weather_Icon_Index_Duststorm;
            break;
        case Weather_Code_Sandstorm:                 //29 - 强沙尘暴
            icon_index = Weather_Icon_Index_Sandstorm;
            break;
        case Weather_Code_Foggy:                     //30 - 雾
            icon_index = Weather_Icon_Index_Foggy;
            break;
        case Weather_Code_Haze:                      //31 - 霾
            icon_index = Weather_Icon_Index_Haze;
            break;
        case Weather_Code_Windy:                     //32 - 风
            icon_index = Weather_Icon_Index_Windy;
            break;
        case Weather_Code_Blustery:                  //33 - 大风
            icon_index = Weather_Icon_Index_Blustery;
            break;
        case Weather_Code_Hurricane:                 //34 - 飓风
            icon_index = Weather_Icon_Index_Hurricane;
            break;
        case Weather_Code_Tropical_Storm:            //35 - 热带风暴
            icon_index = Weather_Icon_Index_Tropical_Storm;
            break;
        case Weather_Code_Tornado:                   //36 - 龙卷风
            icon_index = Weather_Icon_Index_Tornado;
            break;
        case Weather_Code_Cold:                      //37 - 冷
            icon_index = Weather_Icon_Index_Cold;
            break;
        case Weather_Code_Hot:                       //38 - 热
            icon_index = Weather_Icon_Index_Hot;
            break;
        case Weather_Code_Unknown:                   //99 - 未知
            icon_index = Weather_Icon_Index_Unknown;
            break;

        default:
            icon_index = Weather_Icon_Index_Unknown;
            break;
    }

    return icon_index;
}


/**
 * @brief 显示天气图标
 * @param x - 横坐标
 * @param y - 纵坐标
 * @param weather_code - 天气编号
 * @return 如果正常返回 LCD_DISPLAY_OK, 如果返回其他值请查看 Lcd_Diplay_State_e
 */
Lcd_Diplay_State_e hal_lcd_show_weather_icon(uint16_t x, uint16_t y,  uint16_t weather_code)
{
    Weather_Icon_Index_e icon_index = Weather_Icon_Index_Unknown;
    Lcd_Diplay_State_e ret = LCD_DISPLAY_OK;

    //获取图标索引
    icon_index = hal_lcd_get_weather_icon_index(weather_code);

    //区域判断
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
 * @brief 滑动切换主界面
 * @param date - 时间信息, 详情请查看 Date_t, 此信息在 page 0 显示
 * @param weather_code - 天气代码, 详情请查看 Weather_Code_e, 此信息在 page 1 显示
 * @param temperature - 温度, 此信息在 page 1 显示
 * @param main_page_num - 哪一页是主页面(当前正在显示的画面), 该参数只能是0 或者 1, 
 * @return none
 */
void hal_lcd_main_page_slide(Date_t date, Weather_Code_e weather_code, int temperature, uint8_t main_page_num)
{
    uint16_t slide_width, slide_height;   //幻灯片区域大小
    uint16_t x, y; //幻灯片区域起始地址
    uint16_t icon_index = 0;

    uint8_t str_date_buf[30] = {0};
    uint8_t str_time_buf[30] = {0};
    uint8_t str_temperature_buf[30] = {0};

    uint8_t page0_buf[240*80/8] = {0};  //第0页
    uint8_t page1_buf[240*80/8] = {0};  //第1页
    uint16_t buf_offset_1 = 0, buf_offset_2 = 0, buf_offset_3=0, buf_offset_4=0, buf_offset_5=0, buf_offset_6=0;

    uint8_t du[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,
    0x08,0x10,0x08,0x10,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* ° */
    };

    x = 0;
    y = 55;

    if(main_page_num > 1)
        return;

    //幻灯片起始地址
    x = LCD_DATE_SHOW_X;
    y = LCD_DATE_SHOW_Y;

    //幻灯片宽和高
    slide_width = LCD_WIDTH;
    slide_height = 80; //这个高度需要确保全部内容能够正常显示

    //将内容复制到page0
    //复制日期 - 例如 2022-08-14 周日    23:09:25
    sprintf((char *)str_date_buf, "%4d-%02d-%02d     ", date.year, date.month, date.day);
    sprintf((char *)str_time_buf, "%02d:%02d:%02d", date.hour, date.minute, date.second);
    buf_offset_4 = 48*30 + 7;

    buf_offset_5 = 27*30 + 15;
    if(temperature > -100 && temperature < -9)
    {
        //占用3个字节
        sprintf((char *)str_temperature_buf, "%d", temperature);
    }
    else if(temperature > -10 && temperature < 0)
    {
        //温度占用2个字节
        sprintf((char *)str_temperature_buf, " %d", temperature);
    }
    else if(temperature > 0 && temperature < 10)
    {
        //温度占用1个字节
        sprintf((char *)str_temperature_buf, "  %d", temperature);
    }
    else if(temperature > 9)
    {
        //温度占用2个字节
        sprintf((char *)str_temperature_buf, " %d", temperature);
    }

    for (int i = 0; i < 32; i++)
    {
        buf_offset_1 = 30*i;
        buf_offset_2 = 2*i;
        buf_offset_3 = 4*i;
        //按行复制 - 例如: 2022-08-14 周日
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
        memcpy( page0_buf + buf_offset_1 + 20, ascii_3216 + (str_date_buf[10] - 32)*64 + buf_offset_2, 2);//空格
        memcpy( page0_buf + buf_offset_1 + 22, Chinese_Gbk[2].Msk + buf_offset_3, 4);             //周
        memcpy( page0_buf + buf_offset_1 + 26, Chinese_Gbk[date.week + 3].Msk + buf_offset_3, 4); //日

        // vTaskDelay(1);

        //按行复制 - 例如: 23:09:25
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 0,  ascii_3216 + (str_time_buf[0] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 2,  ascii_3216 + (str_time_buf[1] - 32)*64 + buf_offset_2, 2); //3
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 4,  ascii_3216 + (str_time_buf[2] - 32)*64 + buf_offset_2, 2); //:
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 6,  ascii_3216 + (str_time_buf[3] - 32)*64 + buf_offset_2, 2); //0
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 8,  ascii_3216 + (str_time_buf[4] - 32)*64 + buf_offset_2, 2); //9
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 10, ascii_3216 + (str_time_buf[5] - 32)*64 + buf_offset_2, 2); //:
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 12, ascii_3216 + (str_time_buf[6] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page0_buf + buf_offset_4 + buf_offset_1 + 14, ascii_3216 + (str_time_buf[7] - 32)*64 + buf_offset_2, 2); //5

        //按行复制温度 - 例如: 32℃
        //温度共占3个字节, 加上单位 ℃
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 0, ascii_3216 + (str_temperature_buf[0] - 32)*64 + buf_offset_2, 2); //空格
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 2, ascii_3216 + (str_temperature_buf[1] - 32)*64 + buf_offset_2, 2); //3
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 4, ascii_3216 + (str_temperature_buf[2] - 32)*64 + buf_offset_2, 2); //2
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 8, du + buf_offset_2, 2); //°
        memcpy(page1_buf + buf_offset_5 + buf_offset_1 + 10, ascii_3216 + ('C' - 32)*64 + buf_offset_2, 2); //C
    }

    icon_index = hal_lcd_get_weather_icon_index(weather_code);
    buf_offset_6 = 15*30 + 7;
    for (int i = 0; i < 56; i++)
    {
        //按行复制天气图标
        buf_offset_1 = 30*i;
        memcpy(page1_buf + buf_offset_6 + buf_offset_1, &weather_icon_tab[icon_index][i*7], 7);
    }
    
    
    //验证数组是否正确
    //hal_lcd_fill_picture(x, y, slide_width, slide_height, COLOR_WHITE, COLOR_BLACK, page0_buf);
    //hal_lcd_fill_picture(x, y, slide_width, slide_height, COLOR_WHITE, COLOR_BLACK, page1_buf);


    Lcd_Slide_Config_t Lcd_Slide_Config;

    //幻灯片区域
    Lcd_Slide_Config.x = x;
    Lcd_Slide_Config.y = y;
    Lcd_Slide_Config.width = slide_width;
    Lcd_Slide_Config.height = slide_height;

    //滑动方向
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Top;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Bottom;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Left;
    Lcd_Slide_Config.slide_dir = Lcd_Slide_Right;

    //图片颜色 和 图片
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

    //一帧多少秒 和 一帧移动多少像素点
    Lcd_Slide_Config.speed_ms = 1;
    Lcd_Slide_Config.step_len = 8;

    hal_lcd_slide_show(Lcd_Slide_Config);
}

/**
 * @brief 调试用, 后期可删除
 * @param none
 * @return none
 */
void hal_lcd_test(void)
{
#if 0
    //主页面测试
    hal_lcd_show_Date(LCD_DATE_SHOW_X, LCD_DATE_SHOW_Y, 2022, 8, 14, Sunday);
    hal_lcd_show_time(LCD_TIME_SHOW_X, LCD_TIME_SHOW_Y, 23, 9, 25);

    hal_lcd_draw_line(LCD_LINE_X, LCD_LINE_Y, LCD_WIDTH, LCD_LINE_HEIGHT, COLOR_WHITE);

    hal_lcd_show_Small_temperature(LCD_TEMP_SMALL_SHOW_X, LCD_TEMP_SMALL_SHOW_Y, 32);
    hal_lcd_show_small_time(LCD_SMALL_TIME_SHOW_X, LCD_SMALL_TIME_SHOW_Y, 23, 9);
#endif

#if 0
    //副页面测试
    hal_lcd_show_weather_icon(LCD_WEATHER_ICON_X, LCD_WEATHER_ICON_Y, Weather_Code_Mostly_Cloudy);
    hal_lcd_show_temperature(LCD_TEMP_SHOW_X, LCD_TEMP_SHOW_Y, 32);

    hal_lcd_draw_line(LCD_LINE_X, LCD_LINE_Y, LCD_WIDTH, LCD_LINE_HEIGHT, LCD_LINE_COLOR);

    hal_lcd_show_Small_temperature(LCD_TEMP_SMALL_SHOW_X, LCD_TEMP_SMALL_SHOW_Y, 32);
    hal_lcd_show_small_time(LCD_SMALL_TIME_SHOW_X, LCD_SMALL_TIME_SHOW_Y, 9, 30);
#endif

#if 0
    //幻灯片测试
    Lcd_Slide_Config_t Lcd_Slide_Config;

    //幻灯片区域
    Lcd_Slide_Config.x = 50;
    Lcd_Slide_Config.y = 50;
    Lcd_Slide_Config.width = 56;
    Lcd_Slide_Config.height = 56;

    //滑动方向
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Top;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Bottom;
    //Lcd_Slide_Config.slide_dir = Lcd_Slide_Left;
    Lcd_Slide_Config.slide_dir = Lcd_Slide_Right;

    //图片颜色 和 图片
    Lcd_Slide_Config.src_color = COLOR_WHITE;
    Lcd_Slide_Config.src_back_color = COLOR_BLUE;
    Lcd_Slide_Config.src_buf = (uint8_t *)weather_icon_tab[0];

    Lcd_Slide_Config.dest_color = COLOR_WHITE;
    Lcd_Slide_Config.dest_back_color = COLOR_BLUE;
    Lcd_Slide_Config.dest_buf = (uint8_t *)weather_icon_tab[1];

    //一帧多少秒 和 一帧移动多少像素点
    Lcd_Slide_Config.speed_ms = 1;
    Lcd_Slide_Config.step_len = 8;

    printf("%d\n", hal_lcd_slide_show(Lcd_Slide_Config));
#endif

#if 1
    //测试主界面幻灯片
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

