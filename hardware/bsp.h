#ifndef BSP_97F1LVPV
#define BSP_97F1LVPV
#ifdef __cplusplus
extern "C" { /*}*/
#endif

#define USE_SOFT_SPI2           1
#define LOW                     0
#define HIGH                    1
#define CON(_V1, _V2)           _V1##_V2
#define ___P(_v)                CON(GPIO_Pin_, _v)
#define __P(_v)                 ___P(_v)
#define _P(_v)                  __P(_v)

/*
 *
 *
 * 1、蜂鸣器改为PA13
2、电机控制ZF改为PA12，TS改为PA11
3、LED2-2  改为PB7，
4、PA4，PA14为后续电池SMBUS通讯预留。
5、PA0 为新增DS18B20读取外围板温度。 主要是充电温度过高保护。
6、PB0 为单总线ATSHA204A，加密芯片
7、PB1为充电状态指示，输入低表示充电中，变高为充满。
8、PA7为电机转速PWM输入，一转6个脉冲  大致频率是800hz
9、和3399通讯改为UART2
 *
 *
 * */
//GPIOA
//
/*GPIOA OUT*/
#define POWR_DQ_GPIO            GPIOA
#define POWER_DQ_APB2_PERIP     RCC_APB2Periph_GPIOA
#define POWER_DQ_PIN_           1
#define POWER_CTL_PIN_          2
#define BEEP_PIN_               13
#define LED2_1_PIN_             5
#define LED2_3_PIN_             6
/*#define LED1_3_PIN_             7*/
#define MOTOR_CTL_PIN_          8
#define MOTOR_ZF_PIN_           12
#define MOTOR_TS_PIN_           11
#define HEATING0_PIN_           15

#define POWER_DQ_OUT_PIN        PAout(POWER_DQ_PIN_)
#define POWER_CTL_OUT_PIN       PAout(POWER_CTL_PIN_)
#define BEEP_PIN                PAout(BEEP_PIN_)
#define LED2_1_PIN              PAout(LED2_1_PIN_)
#define LED2_3_PIN              PAout(LED2_3_PIN_)
/*#define LED1_3_PIN              PAout(LED1_3_PIN_)*/
#define MOTOR_CTL_PIN           PAout(MOTOR_CTL_PIN_)
#define MOTOR_TS_PIN            PAout(MOTOR_TS_PIN_)
#define MOTOR_ZF_PIN            PAout(MOTOR_ZF_PIN_)
#define HEATING0_PIN            PAout(HEATING0_PIN_)

#define BEEP_IN_PIN             PAin(BEEP_PIN_)

#define GPIOA_OUTs              ( _P(POWER_CTL_PIN_) | \
                                _P(BEEP_PIN_) | \
                                _P(LED2_1_PIN_) | \
                                _P(LED2_3_PIN_) | \
                                _P(MOTOR_CTL_PIN_) | \
                                _P(MOTOR_TS_PIN_) | \
                                _P(MOTOR_ZF_PIN_) | \
                                _P(HEATING0_PIN_) | \
                                0)

/*GPIOA IN*/
#define TEMPSENSOR2_PIN_        0
#define MCU_AD_PIN_             4
#define UART1_RX_PIN_           10
#define MOTOR_PWM_SPD_PIN_      7


#define MCU_AD_PIN              PAin(MCU_AD_PIN_)
#define POWER_DQ_IN_PIN         PAin(POWER_DQ_PIN_)
#define TEMPSENSOR2_IN_PIN      PAin(TEMPSENSOR2_PIN_)


#define LED2_1_IN_PIN           PAin(LED2_1_PIN_)
#define LED2_3_IN_PIN           PAin(LED2_3_PIN_)
#define MOTOR_PWM_SPD_IN_PIN    PAin(MOTOR_PWM_SPD_PIN_)

#define GPIOA_INs               (_P(MCU_AD_PIN_) | \
                                _P(UART1_RX_PIN_) | \
                                _P(TEMPSENSOR2_PIN_) | \
                                _P(POWER_DQ_PIN_) | \
                                _P(MOTOR_PWM_SPD_PIN_) |\
                                0)





//GPIOB OUT

#define LED1_1_PIN_             0
/*#define LED1_2_PIN_             1*/
#define I2C_SCL_PIN_            3
#define I2C_SDA_PIN_            4
#define PWR_12V_ENABLE_PIN_     5//无线充电12V电压控制
#define LED2_2_PIN_             7
#define SYS_PWR_HOLD_PIN_       9
#define SPI2_CS_PIN_            12
#define SPI2_CLK_PIN_           13
#if USE_SOFT_SPI2
#else
#define SPI2_MISO_PIN_          14
#endif
#define SPI2_MOSI_PIN_          15

#define LED1_1_PIN              PBout(LED1_1_PIN_)
#define LED1_2_PIN              PBout(LED1_2_PIN_)
#define PWR_12V_ENABLE_PIN      PBout(PWR_12V_ENABLE_PIN_)
#define LED2_2_PIN              PBout(LED2_2_PIN_)
#define SYS_PWR_HOLD_PIN        PBout(SYS_PWR_HOLD_PIN_)
#define SPI2_CS_PIN             PBout(SPI2_CS_PIN_)
#define SPI2_CLK_PIN            PBout(SPI2_CLK_PIN_)
#if USE_SOFT_SPI2
#else
#define SPI2_MISO_PIN           PBout(SPI2_MISO_PIN_)
#endif
#define SPI2_MOSI_PIN           PBout(SPI2_MOSI_PIN_)

#define I2C_SCL_GPIO_PIN        _P(I2C_SCL_PIN_)
#define I2C_SDA_GPIO_PIN        _P(I2C_SDA_PIN_)

#define I2C_OUT                 PBout
#define I2C_IN                  PBin

#define GPIO_PORT_I2C	        GPIOB
#define RCC_I2C_PORT 	        RCC_APB2Periph_GPIOB


#if USE_SOFT_SPI2
#define GPIOB_OUTs              (_P(LED1_1_PIN_)     | \
                                _P(PWR_12V_ENABLE_PIN_) | \
                                _P(LED2_2_PIN_) | \
                                _P(SYS_PWR_HOLD_PIN_) | \
                                _P(SPI2_CS_PIN_) | \
                                _P(SPI2_CLK_PIN_) | \
                                _P(SPI2_MOSI_PIN_) | \
                                0)
#else
#define GPIOB_OUTs              (_P(LED1_1_PIN_)     | \
                                _P(PWR_12V_ENABLE_PIN_) | \
                                _P(LED2_2_PIN_) | \
                                _P(SYS_PWR_HOLD_PIN_) | \
                                _P(SPI2_CS_PIN_) | \
                                _P(SPI2_CLK_PIN_) | \
                                _P(SPI2_MISO_PIN_) | \
                                _P(SPI2_MOSI_PIN_) | \
                                0)
#endif

//GPIOB IN

#define PWR_12V_SENSE_PIN_      1
#define PWR_KEY_DET_PIN_        8
#define COLLISION_HEAD_PIN_     10
#define COLLISION_TAIL_PIN_     11
#define SPI_MISO_PIN_           14

#define PWR_12V_SENSE_PIN       PBin(PWR_12V_SENSE_PIN_)
#define PWR_KEY_DET_PIN         PBin(PWR_KEY_DET_PIN_)
#define COLLISION_HEAD_PIN      PBin(COLLISION_HEAD_PIN_)
#define COLLISION_TAIL_PIN      PBin(COLLISION_TAIL_PIN_)
#define SPI_MISO_PIN            PBin(SPI_MISO_PIN_)

#define GPIOB_INs               (_P(PWR_KEY_DET_PIN_) |\
                                _P(COLLISION_HEAD_PIN_) | \
                                _P(COLLISION_TAIL_PIN_) | \
                                _P(SPI_MISO_PIN_) | \
                                0)

#define LED2_2_IN_PIN           PBin(LED2_2_PIN_)


#define TEMPSENSOR0_PIN_        13
#define TEMPSENSOR1_PIN_        14
//GPIOC OUT

#define HEATING1_PIN_           15

#define HEATING1_PIN            PCout(HEATING1_PIN_)

#define TEMPSENSOR0_OUT_PIN      PCout(TEMPSENSOR0_PIN_)
#define TEMPSENSOR1_OUT_PIN      PCout(TEMPSENSOR1_PIN_)

#define GPIOC_OUTs              (_P(HEATING1_PIN_)     | \
                                0)
//GPIOC IN


#define TEMPSENSOR0_IN_PIN      PCin(TEMPSENSOR0_PIN_)
#define TEMPSENSOR1_IN_PIN      PCin(TEMPSENSOR1_PIN_)

#define GPIOC_INs               (0)

#define GPIOD_OUTs              0



#define INTTERUPT_ENABLE()      __set_PRIMASK(0)
#define INTTERUPT_DISABLE()     __set_PRIMASK(1)

#define LED2_ON                 HIGH
#define LED2_OFF                LOW

void bsp_init(void);

void iwdg_init(unsigned char prer, int rlr);

void iwdg_feed(void);

uint8_t spi2_readwrite(uint8_t data);

void spi2_init(void);

void spi2_uninit(void);

#define IIC_SCL                 I2C_OUT(I2C_SCL_PIN_)
#define IIC_SDA                 I2C_OUT(I2C_SDA_PIN_)

#define READ_SDA                I2C_IN(I2C_SDA_PIN_)


#define REBOOT()                do { WDG_INIT(); delay_ms(10); while (1) ; } while(0)
/*#define REBOOT()                do { WDG_INIT(); delay_ms(10); while (1) NVIC_SystemReset(); } while(0)*/
#define WDG_INIT()              iwdg_init(4, 1000)
#define WDG_FEED()              iwdg_feed()

#if 0
#define BEEP_ON()               beep_on_ex(HIGH)
#define BEEP_OFF()              beep_on_ex(LOW)

#else
#define BEEP_ON()               (BEEP_PIN = HIGH)
#define BEEP_OFF()              (BEEP_PIN = LOW)
#endif

#define BTN_NORMAL_VAL          HIGH
#define BTN_HEAD_TRIG_VAL       LOW
#define BTN_TAIL_TRIG_VAL       LOW

#define LED_RED_PIN             LED2_1_PIN
#define LED_GREEN_PIN           LED2_2_PIN
#define LED_BLUE_PIN            LED2_3_PIN

#define LED_RED_IN_PIN          LED2_1_IN_PIN
#define LED_GREEN_IN_PIN        LED2_2_IN_PIN
#define LED_BLUE_IN_PIN         LED2_3_IN_PIN

#define LED_RED_ON()            (LED_RED_PIN = LOW)
#define LED_GREEN_ON()          (LED_GREEN_PIN = LOW)
#define LED_BLUE_ON()           (LED_BLUE_PIN = LOW)

#define LED_RED_OFF()           (LED_RED_PIN = HIGH)
#define LED_GREEN_OFF()         (LED_GREEN_PIN = HIGH)
#define LED_BLUE_OFF()          (LED_BLUE_PIN = HIGH)

#define LED_RED_FLASH()         (LED_RED_PIN = !LED_RED_IN_PIN)
#define LED_GREEN_FLASH()       (LED_GREEN_PIN = !LED_GREEN_IN_PIN)
#define LED_BLUE_FLASH()        (LED_BLUE_PIN = !LED_BLUE_IN_PIN)
#define LED_ALL_OFF()           { LED_RED_OFF(); LED_GREEN_OFF(); LED_BLUE_OFF(); }


#define HEATING0_INPUT_PIN      (PAin(HEATING0_PIN_)
#define HEATING1_INPUT_PIN      (PCin(HEATING1_PIN_)

#define HEATING0_ON()           (HEATING0_PIN = HIGH)
#define HEATING0_OFF()          (HEATING0_PIN = LOW)

#define HEATING1_ON()           (HEATING1_PIN = HIGH)
#define HEATING1_OFF()          (HEATING1_PIN = LOW)

#define HEATING0_IS_ON()        (HEATING0_INPUT_PIN == HIGH)
#define HEATING1_IS_ON()        (HEATING1_INPUT_PIN == HIGH)

#define POWER_HOLD_ON()         (SYS_PWR_HOLD_PIN = HIGH)
#define POWER_HOLD_OFF()        (SYS_PWR_HOLD_PIN = LOW)
#define CHARGE_START()          (PWR_12V_ENABLE_PIN = HIGH)
#define CHARGE_STOP()           (PWR_12V_ENABLE_PIN = LOW)

#define HEATING0_TEMP_MAX       530
#define HEATING1_TEMP_MAX       530

#define MOTOR_CURRENT_BLOCK     555
#define SYS_CURRENT_NORMAL      165
#define SYS_3399_CURRENT_NORMAL 600
#define HEATING0_CURRENT_MAX    400
#define HEATING1_CURRENT_MAX    400

#ifndef BIT
#define BIT(_v)                 (1 << (_v))
#endif

enum DEV_STATUS {
    DEV_MPU6050 = BIT(0),
    DEV_DS18B20_0 = BIT(1),
    DEV_DS18B20_1 = BIT(2),
    DEV_DS2781 = BIT(3),
    DEV_RC522 = BIT(4),
    DEV_INA219  = BIT(5),
};

#define DEFINE_DEV_REF(_V)              //static uint8_t _V
#define DEV_GET_REF(_V)                 //(++_V)
#define DEV_PUT_REF(_V)                 //(--_V)
#define DEV_REF(_V)                     0

#if UART1_DISP_ENABLE
#define mid_send_to_host(...)           { printf(__VA_ARGS__); printf2(__VA_ARGS__); }
#else
#define mid_send_to_host(...)           printf(__VA_ARGS__)
#endif
#define mid_send_to_host_c(_V)         fputc(_V, 0)

void dev_chk(void);

void dev_self_check(void);

void beep_on_ex(uint8_t val);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: BSP_97F1LVPV */
