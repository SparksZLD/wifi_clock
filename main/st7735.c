#include "st7735.h"

/***************************************** st7735 gpio define ****************************************************/

/* 选择硬件SPI 或者 软件SPI。 注意:只能选择一个 */
//#define LCD_SOFTWARE_SPI            1
#define LCD_HARDWARE_SPI          1

/* 选择SPI引脚。 注意:只能选择一个 */
#define CONFIG_ST7735_VSPI_HOST     1
//#define CONFIG_ST7735_HSPI_HOST   1

#if CONFIG_ST7735_VSPI_HOST    
    #define ST7735_SPI_HOST   VSPI_HOST
    #define PIN_NUM_MISO      -1
    #define PIN_NUM_MOSI      23  // SDA
    #define PIN_NUM_CLK       18
    #define PIN_NUM_CS        5

    #define PIN_NUM_DC        21
    #define PIN_NUM_RST       22
    #define PIN_NUM_BCKL      4
#elif CONFIG_ST7735_HSPI_HOST   //使用HSPI
    #define ST7735_SPI_HOST   HSPI_HOST
    #define PIN_NUM_MISO      -1
    #define PIN_NUM_MOSI      13  // SDA
    #define PIN_NUM_CLK       14
    #define PIN_NUM_CS        15

    #define PIN_NUM_DC        4
    #define PIN_NUM_RST       2
#endif


#if LCD_HARDWARE_SPI
    //如果使用硬件SPI, 设置波特率
    #define LCD_HARDWARE_SPI_BPS (50 * 1000 * 1000)
#endif



/***************************************** st7735 gpio define end ****************************************************/


/***************************************** st7735 params define ****************************************************/
static const char *TAG = "LCD_DEBUG";
// RGB-565 16bit, 240*240;
//static uint8_t display_buff[LCD_WIDTH * LCD_HEIGHT * 2];

#ifdef LCD_HARDWARE_SPI
static spi_device_handle_t spi_dev;
#endif

/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;


//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {0x36, {0x00}, 1}, //正向
    //{0x36, {0x40}, 1},
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x05}, 1}, //RGB565
    /* Porch Setting */
    {0xB2, {0x0C, 0x0C, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x35}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x19}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01}, 1},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x12}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0F}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 2},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}, 14},
    /* Negative Voltage Gamma Control */
    {0xE1, {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}, 14},
    /* This command is used to enter into display inversion mode */
    {0x21, {0}, 0x80}, //颜色正常显示
    /* This command is used to recover from display inversion mode */
    //{0x20, {0}, 0x80}, //颜色反转
    /* Sleep Out */
    {0x11, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
    /* end */
    {0, {0}, 0xff}
};


/***************************************** st7735 params define end****************************************************/

#ifdef LCD_HARDWARE_SPI
// This function is called (in irq context!) just before a transmission starts. It will
// set the D/C line to the value indicated in the user field.
static void st7735_spi_pre_transfer_callback(spi_transaction_t *t) 
{
  int dc = (int)t->user;
  gpio_set_level(PIN_NUM_DC, dc);
}
#endif

static void st7735_gpio_init(void)
{
    gpio_reset_pin(PIN_NUM_MOSI);
    gpio_reset_pin(PIN_NUM_CLK);
    gpio_reset_pin(PIN_NUM_CS);

    gpio_reset_pin(PIN_NUM_DC);
    gpio_reset_pin(PIN_NUM_RST);
    gpio_reset_pin(PIN_NUM_BCKL);

#ifdef LCD_SOFTWARE_SPI
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_NUM_DC);
    gpio_set_level(PIN_NUM_DC, 1);

    gpio_pad_select_gpio(PIN_NUM_RST);
    gpio_set_level(PIN_NUM_RST, 1);

    gpio_pad_select_gpio(PIN_NUM_BCKL);
    gpio_set_level(PIN_NUM_BCKL, 1);

    gpio_pad_select_gpio(PIN_NUM_MOSI);
    gpio_set_level(PIN_NUM_MOSI, 1);

    gpio_pad_select_gpio(PIN_NUM_CS);
    gpio_set_level(PIN_NUM_CS, 1);

    gpio_pad_select_gpio(PIN_NUM_CLK);
    gpio_set_level(PIN_NUM_CLK, 1);

    ESP_LOGI(TAG, "LCD software spi gpio init finished...");
#endif

#ifdef LCD_HARDWARE_SPI
    // Initialize non-SPI GPIOs
    gpio_pad_select_gpio(PIN_NUM_DC);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_NUM_RST);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_NUM_BCKL);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(PIN_NUM_CS);
    gpio_set_level(PIN_NUM_CS, 1);

    ESP_LOGI(TAG, "LCD hardware spi gpio init finished...");
#endif
}


static void st7735_spi_init(void)
{
#ifdef LCD_HARDWARE_SPI
    esp_err_t ret;
    
    // esp32 spi configuration
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,  // unused
        .quadhd_io_num = -1,  // unused
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = LCD_HARDWARE_SPI_BPS,      //bps
        .mode = 3,                                   //SPI mode 3, SPI 空闲时, 时钟的输出为高电平, 数据在 SPI 下降沿变化，在上升沿采样
        .spics_io_num = PIN_NUM_CS,                  //CS pin
        .queue_size = 7,                             //We want to be able to queue 7 transactions at a time
        .pre_cb = st7735_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };

    // Initialize the SPI bus
    //ret = spi_bus_initialize(ST7735_SPI_HOST, &buscfg, 1);
    ret = spi_bus_initialize(ST7735_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(ST7735_SPI_HOST, &devcfg, &spi_dev);
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "LCD hardware spi init finished...");
#endif
}


#ifdef LCD_SOFTWARE_SPI
void lcd_write_bus(uint8_t dat)   //串行数据写入
{	
	uint8_t i;			  
  
	for(i=0; i<8; i++)
	{			  
        gpio_set_level(PIN_NUM_CLK, 0);
		if((dat&0x80) == 0x80)
           gpio_set_level(PIN_NUM_MOSI, 1);
		else 
           gpio_set_level(PIN_NUM_MOSI, 0);
		gpio_set_level(PIN_NUM_CLK, 1);
		dat<<=1;   
	}	
    gpio_set_level(PIN_NUM_CLK, 0);		
}


void lcd_write_8_data(uint8_t da) //发送数据-8位参数
{	
    gpio_set_level(PIN_NUM_CS, 0);
    gpio_set_level(PIN_NUM_DC, 1);
	lcd_write_bus(da);  
    gpio_set_level(PIN_NUM_CS, 1);
}


void lcd_write_data(uint8_t *buf, uint32_t len)
{
    gpio_set_level(PIN_NUM_CS, 0);
    gpio_set_level(PIN_NUM_DC, 1);
    for (int i = 0; i < len; i++)
    {
        lcd_write_bus(buf[i]);  
    }
    gpio_set_level(PIN_NUM_CS, 1);
}
#endif


/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
static void st7735_cmd(const uint8_t cmd) 
{
#ifdef LCD_HARDWARE_SPI
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = 8;                                    // Command is 8 bits
    t.tx_buffer = &cmd;                              // The data is the cmd itself
    t.user = (void *)0;                              // D/C needs to be set to 0
    ret = spi_device_polling_transmit(spi_dev, &t);  // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.

    //ESP_LOGI(TAG, "LCD hardware spi send cmd:0x%x", cmd);
#endif

#ifdef LCD_SOFTWARE_SPI
    gpio_set_level(PIN_NUM_CS, 0);
    gpio_set_level(PIN_NUM_DC, 0);
    lcd_write_bus(cmd);
    gpio_set_level(PIN_NUM_CS, 1);
    //ESP_LOGI(TAG, "LCD software spi send cmd:0x%x", cmd);
#endif
}


/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void st7735_data(const uint8_t *data, int len) 
{
#ifdef LCD_HARDWARE_SPI
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) return;                            // no need to send anything
    memset(&t, 0, sizeof(t));                        // Zero out the transaction
    t.length = len * 8;                              // Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;                              // Data
    t.user = (void *)1;                              // D/C needs to be set to 1
    ret = spi_device_polling_transmit(spi_dev, &t);  // Transmit!
    assert(ret == ESP_OK);                           // Should have had no issues.
    //ESP_LOGI(TAG, "LCD hardware spi send data");
#endif

#ifdef LCD_SOFTWARE_SPI
    lcd_write_data((uint8_t*)data, len);
    //ESP_LOGI(TAG, "LCD software spi send data");
#endif
}


static void st7735_reg_init(void)
{
    // Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(500 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(500 / portTICK_RATE_MS);

    // Send all the init commands
    int cmd = 0;
    while (st_init_cmds[cmd].databytes != 0xff) 
    {
        st7735_cmd(st_init_cmds[cmd].cmd);
        st7735_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes & 0x1F);
        if (st_init_cmds[cmd].databytes & 0x80) 
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }
}


void lcd_address_window_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t send_buf[4];

    st7735_cmd(0x2a);
    send_buf[0] = x1 >> 8;
    send_buf[1] = x1;
    send_buf[2] = x2 >> 8;
    send_buf[3] = x2;
    st7735_data(send_buf, 4);

    st7735_cmd(0x2b);
    send_buf[0] = y1 >> 8;
    send_buf[1] = y1;
    send_buf[2] = y2 >> 8;
    send_buf[3] = y2;
    st7735_data(send_buf, 4);

    st7735_cmd(0x2c);
}


void st7735_backlight_set(uint8_t x)
{
    if(x>0)
        x = 1;
    gpio_set_level(PIN_NUM_BCKL, x);
}


void lcd_init(void)
{
    st7735_gpio_init();
    st7735_spi_init();
    st7735_reg_init();
    lcd_clear(COLOR_BLACK); //黑色
    st7735_backlight_set(1);
}


void lcd_clear(uint16_t color)
{
    uint8_t display_buff[LCD_WIDTH * LCD_HEIGHT * 2];
    for (int i = 0; i < (LCD_WIDTH * LCD_HEIGHT * 2); i = i + 2)
    {
        display_buff[i] = (uint8_t)(color >> 8);
        display_buff[i + 1] = (uint8_t)(color & 0xFF);
    }

  lcd_address_window_set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  st7735_data(display_buff, LCD_WIDTH * LCD_HEIGHT * 2);
}



void lcd_test(void)
{
    static uint16_t aa[3] ={0xF800, 0x07E0, 0x001F};
    static uint16_t i = 0;
    lcd_clear(aa[i]);
    printf("0x%x\n", aa[i]);
    if(++i >= 3)
        i = 0;
    
    vTaskDelay(1000);
}


