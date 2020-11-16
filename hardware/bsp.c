#include "sys.h"
#include "main.h"
#include "bsp.h"
#include "system.h"
#include "bsp_i2c.h"
#include "timer.h"
#include "motor.h"
#include "rc522.h"
#include "ds2781.h"
#include "ina219.h"
#include "mpu6050.h"
#include "ds18b20.h"
void motor_init(void);

void delay_us_ex(uint16_t time)
{
   int i = 0;
   while(time--) {
      i = 10;  //自己定义
      while(i--) ;
   }
}
void delay_ms_ex(int time)
{
   int i = 0;
   while(time--) {
      i = 12000;
      while(i--);
   }
}


static void gpio_init(void)
{
    GPIO_InitTypeDef  def;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB |\
            RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    /*
     *pb3 pb4 must set
     * JTAG-DP Disabled and SW-DP Enabled **/
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

    def.GPIO_Pin = GPIO_Pin_All;
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &def);
    GPIO_Init(GPIOB, &def);
    GPIO_Init(GPIOC, &def);
    GPIO_Init(GPIOD, &def);

    def.GPIO_Pin = GPIOA_OUTs;
    def.GPIO_Mode = GPIO_Mode_Out_PP;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &def);

    def.GPIO_Pin = GPIOB_OUTs;
    GPIO_Init(GPIOB, &def);

    def.GPIO_Pin = GPIOC_OUTs;
    GPIO_Init(GPIOC, &def);

    def.GPIO_Pin = GPIOD_OUTs;
    GPIO_Init(GPIOD, &def);
    GPIO_SetBits(GPIOD, GPIOD_OUTs);

    def.GPIO_Pin = GPIOA_INs;
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    def.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &def);

    def.GPIO_Pin = GPIOB_INs;
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    def.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &def);


    def.GPIO_Pin = GPIOC_INs;
    /*def.GPIO_Mode = GPIO_Mode_IN_FLOATING;*/
    def.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &def);


    def.GPIO_Pin = _P(PWR_12V_SENSE_PIN_);
    def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &def);

    MOTOR_CTL_DISABLE();
    delay_ms_ex(200);
    POWER_HOLD_ON();
    LED_ALL_OFF();

    HEATING0_OFF();
    HEATING1_OFF();

    CHARGE_STOP();
    /*CHARGE_START();*/
}


/*Tout=((4*2^prer)*rlr)/40 (ms).*/
void iwdg_init(unsigned char prer, int rlr)
{
    IWDG->KR = 0X5555;
    IWDG->PR = prer;
    IWDG->RLR = rlr;
    IWDG->KR = 0XAAAA;
    IWDG->KR = 0XCCCC;
}
void iwdg_feed(void)
{
    IWDG->KR = 0XAAAA;//reload
}


void spi2_init(void)
{
#if USE_SOFT_SPI2
    /*
    GPIO_InitTypeDef def;
    //配置 CS 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    def.GPIO_Pin=GPIO_Pin_12;
    def.GPIO_Speed=GPIO_Speed_50MHz;
    def.GPIO_Mode = GPIO_Mode_Out_PP;    //B组12   推挽输出
    GPIO_Init(GPIOB, &def);

    //配置SCK 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
    def.GPIO_Pin = GPIO_Pin_13;
    def.GPIO_Speed = GPIO_Speed_50MHz; //B组12   推挽输出
    def.GPIO_Mode =GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &def);

    //配置MOSI 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
    def.GPIO_Pin =GPIO_Pin_15;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    def.GPIO_Mode =GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &def);

    //配置MISO 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
    def.GPIO_Pin =GPIO_Pin_14;
    def.GPIO_Mode =GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &def);	
    //配置  RST
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
    def.GPIO_Pin = GPIO_Pin_8;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    def.GPIO_Mode =GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &def);*/
#else
    /*GPIOB 12 13 14 15*/
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;        //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //串行同步时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;    //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //定义波特率预分频的值:波特率预分频值为256
    //SPI_BaudRatePrescaler_256 256分频 (SPI 281.25K@sys 72M)
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
    SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
    SPI2_CLK_PIN = SPI2_MISO_PIN  = SPI2_MOSI_PIN = HIGH;

    SPI_Cmd(SPI2, ENABLE); //使能SPI外设
#endif
}

void spi2_uninit(void)
{

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);
    /* Release SPI2 from reset state */
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);
}

uint8_t spi2_readwrite(uint8_t data)
{
    while((SPI2->SR & 0X02) == 0);        //等待发送区空
    SPI2->DR = data;                     //发送一个byte
    while((SPI2->SR & 0X01) == 0);      //等待接收完一个byte
    return SPI2->DR;                  //返回收到的数据
}

static uint8_t chk_count;
//#define HEATING0_TEMP_MAX       530
unsigned int Heat_Temp_MAX = HEATING0_TEMP_MAX;		//加热板加热最高温度
void _dev_self_check(void);
uint16_t get_sys_temp_(void);
void dev_chk(void)
{
    static uint8_t count;
    static short temp;
    if (get_dev_status() & DEV_MPU6050) {
        move_det();
    } else {
        dbg_printf("mpu6050 ng!"CRLF);
    }
    if (get_dev_status() & DEV_DS2781) {
        if (count == 0) {
            ds2781_mutex_lock();
            get_coulomb_data();
            ds2781_mutex_unlock();
        }
        count = (count + 1) & 0x1f;
    } else {
        dbg_printf("dev_ds2781 Ng!"CRLF);
        ;
    }
    if (get_dev_status() & DEV_INA219) {
        current_det();
        ;
    } else {
        dbg_printf("dev_ina219 Ng!"CRLF);
    }
    if (get_dev_status() & DEV_DS18B20_0) {
        temp = ds18b20_0_get_temp();
        dbg_printf("DEV_DS18B20_0:%d"CRLF, temp);
        if (!get_sys_is_charging() && temp < Heat_Temp_MAX && \
                get_sys_bat_low_alarm() < BAT_LOW_ALARM_ERR) {
            /*dbg_printf("heat0 on"CRLF);*/
            HEATING0_ON();
        } else {
            dbg_printf("heat0 off"CRLF);
            HEATING0_OFF();
        }
    } else {
        /*dbg_printf("DS18B20_0 failed!"CRLF);*/
    }
    if (get_dev_status() & DEV_DS18B20_1) {
        temp = ds18b20_1_get_temp();
        dbg_printf("DEV_DS18B20_1:%d"CRLF, temp);
        if (!get_sys_is_charging() && temp < Heat_Temp_MAX && \
                get_sys_bat_low_alarm() < BAT_LOW_ALARM_ERR) {
            /*dbg_printf("heat1 on"CRLF);*/
            HEATING1_ON();
        } else {
            dbg_printf("heat1 off"CRLF);
            HEATING1_OFF();
        }
    } else {
        /*dbg_printf("DS18B20_1 failed"CRLF);*/
    }
    get_sys_volt() = get_sys_volt_();
    get_sys_current_total() = get_sys_current_total_();
    get_sys_quantity() = get_sys_quantity_();
    mid_send_to_host("BatteryStatus_%d_%d_%d_%d!\n", get_sys_volt(), get_sys_temp_(),
            get_sys_quantity(), get_sys_current_total());
    if (chk_count > 0) {
        _dev_self_check();
        --chk_count;
    }
}

void dev_self_check(void)
{
    ++chk_count;
}
uint8_t rfid_is_reading(void);

void dev_failed_led_act(void);
void _dev_self_check(void)
{
    get_dev_failed() = 0;
    if (mpu6050_self_check())
        set_dev_failed(DEV_MPU6050);
    /*if (!ret)
        set_dev_status(DEV_MPU6050);*/
    if (ds2781_self_check()) {
        set_dev_failed(DEV_DS2781);
        mid_send_to_host("BatDatDet self-check fail"CRLF);
    } else {
        mid_send_to_host("BatDatDet self-check ok"CRLF);
    }
    /*if (!ret)
        set_dev_status(DEV_DS2781);*/
    if (ina219_self_check())
        set_dev_failed(DEV_INA219);
    /*if (!ret)
        set_dev_status(DEV_INA219);*/
    if (DS18B20_0_self_check())
        set_dev_failed(DEV_DS18B20_0);
    if (DS18B20_1_self_check())
        set_dev_failed(DEV_DS18B20_1);
    if (rc522_self_check())
        set_dev_failed(DEV_RC522);

    mid_send_to_host("Version:");
    mid_send_to_host(VERSION);
    mid_send_to_host(CRLF);

    mid_send_to_host("chkcode:%x"CRLF, get_dev_failed());
    if (get_dev_failed()) {
        dev_failed_led_act();
    } else {
        get_led_0_sta() = 0;
    }
}
__attribute__((unused))
static test_delay(void)
{
    int i;
    for (i = 1; i; i++) {
        /* code */
        dbg_printf("%d\t", i);
        delay_ms(1000);
    }
}



 



void bsp_init(void)
{
    void *memset(void *s, int c, size_t n);
    memset(&sys_parm, 0, sizeof(sys_parm));
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
    /*set_dev_status(DEV_RC522 | DEV_MPU6050 | DEV_DS2781 | \
            DEV_INA219 | DEV_DS18B20_0 | DEV_DS18B20_1);*/
    gpio_init();
    uart_init(BAUD);
    tim3_int_init(1, SystemCoreClock / (HZ * 2) - 1);
    tick_init();
    i2c_init();
    motor_init();
#if 1

    if (!rc522_init()) {
        set_dev_status(DEV_RC522);
        mid_send_to_host("rc522_init ok"CRLF);
    } else {
        dbg_printf("rc522_init failed"CRLF);
    }
    if (!ds18b20_0_init()) {
        set_dev_status(DEV_DS18B20_0);
        mid_send_to_host("ds18b20_0 init ok"CRLF);
    } else {
        dbg_printf("ds18b20_0_init failed"CRLF);
    }
    if (!ds18b20_1_init()) {
        set_dev_status(DEV_DS18B20_1);
        mid_send_to_host("ds18b20_1 init ok"CRLF);
    } else {
        dbg_printf("ds18b20_1_init failed"CRLF);
    }
    if (!ds2781_init()) {
        set_dev_status(DEV_DS2781);
        mid_send_to_host("ds2781 init ok"CRLF);
    } else {
        dbg_printf("ds2781 init failed!"CRLF);
    }
    if (!ina_init()) {
        set_dev_status(DEV_INA219);
        mid_send_to_host("ina219 init ok"CRLF);
    } else {
        dbg_printf("ina219 init failed!"CRLF);
    }
#if 1
    if (!MPU_Init()) {
        mid_send_to_host("mpu init ok."CRLF);
        set_dev_status(DEV_MPU6050);
    } else {
        dbg_printf("mpu init failed."CRLF);
    }
    dev_self_check();
#endif
#endif
}
