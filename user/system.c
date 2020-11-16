#include "sys.h"
#include "key.h"
#include "protocol.h"
#include "system.h"
#include "motor.h"
#include "ds2781.h"
static void sys_key_init(void);
static unsigned char task_dev(void);
static unsigned char task_sys(void);
static unsigned char task_rfid(void);
#if DEBUG
#define dbg_sys_parm            dbg_printf
static uint8_t task_test(void);
#else
#define dbg_sys_parm(...)
#endif

struct sys_parm_t sys_parm;

enum {
    HEAD_KEY,/*BIT(0)*/
    TAIL_KEY,/*BIT(1)*/
    PWR_DET_KEY,
    PWR_12V_SENSE_KEY,
    KEY_MAX,
};

__attribute__((unused))
static void key_test_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);//

} 
__attribute__((unused))
static uint8_t get_test_key(void)
{
    uint8_t tmp;
    tmp = PCin(5);
    return tmp;
}
__attribute__((unused))
static uint8_t get_bt_head_key(void)
{
    return COLLISION_HEAD_PIN;
}
static uint8_t get_bt_tail_key(void)
{
    return COLLISION_TAIL_PIN;
}
static uint8_t get_bt_pwrdet_key(void)
{
    return PWR_KEY_DET_PIN;
}

static uint8_t get_pwr_12v_sense_key(void)
{
    if (get_sys_current_total() > 100)
        return HIGH;
    else
        return LOW;
}
#pragma pack(1)
struct key_t key_data[KEY_MAX];
#pragma pack()
void report_shutdown(void)
{
    set_buzzer_on();
    set_power_off_led_act();
    while(1) {
        delay_ms(1000); delay_ms(1000); delay_ms(1000); delay_ms(1000);delay_ms(1000);
        WDG_FEED();
        delay_ms(1000); delay_ms(1000); delay_ms(1000); delay_ms(1000);delay_ms(1000);
        set_buzzer_off();
        mid_send_to_host("\nshutdown\n");
    }
}

#define report_collision(_v)                    { mid_send_to_host(_v);\
    mid_send_to_host(_v); mid_send_to_host(_v);mid_send_to_host(_v);\
    mid_send_to_host(_v); mid_send_to_host(_v); mid_send_to_host(_v); }

void delay_ms_ex(int time);
void poweroff(uint8_t delay_s)
{
    uint8_t i;
    loading_close();
    BEEP_ON();
    i = 0;
    mid_send_to_host("shutdown."CRLF);
    while (i++ < delay_s) {
        handle_led(BIT_LED_RED_ON);
        delay_ms(1000);
        WDG_FEED();
    }
    __disable_irq();
    mid_send_to_host("poweroff"CRLF);
    WDG_FEED();
    POWER_HOLD_OFF();
    while (1);
}
static void key_handle(void *parm)
{
    static uint8_t cflag;
    struct key_t *btn = (struct key_t *)parm;
    #define bmark            get_key_mark(btn)
    switch (get_key_event(btn)) {
    case PRESS_DOWN:
        if (bmark == HEAD_KEY) {
            cflag |= HEAD_KEY;
            motor_stop();
            dbg_printf("HEAD_KEY down"CRLF);
            mid_send_to_host("Car_happen_head_collision!\n");
        }
        if (bmark == TAIL_KEY) {
            cflag |= TAIL_KEY;
            motor_stop();
            mid_send_to_host("Car_happen_tail_collision!\n");
        }
        if (bmark == PWR_DET_KEY) {
            dbg_printf("PWR_DET_KEY down"CRLF);
        }
        if (cflag & (HEAD_KEY | TAIL_KEY) == (HEAD_KEY | TAIL_KEY)) {
            /*report_shutdown();*/
            ;
        }
        break;
    case PRESS_UP:
        if (bmark == HEAD_KEY) {
            cflag &= ~HEAD_KEY;
            dbg_printf("HEAD_KEY up"CRLF);
            report_collision("Car_happen_head_collision_end!\n");
        }
        if (bmark == TAIL_KEY) {
            cflag &= ~TAIL_KEY;
            report_collision("Car_happen_tail_collision_end!\n");
        }
        if (bmark == PWR_DET_KEY) {
            dbg_printf("PWR_DET_KEY up"CRLF);
            if (cflag & PWR_DET_KEY) {
                cflag &= ~PWR_DET_KEY;
                poweroff(5);
            }
        }
        if (bmark == PWR_12V_SENSE_KEY) {
            dbg_printf("PWR_12V_SENSE_KEY up"CRLF);
            set_buzzer_freq(5);
            mid_send_to_host("Charge stop\n");
            set_sys_is_charging(0);
            CHARGE_STOP();
            CLR_LED_STATUS(BIT_LED_GREEN_FLASH);
        }
        break;
    case PRESS_LONG_DOWN:
        if (bmark == PWR_12V_SENSE_KEY) {
            set_charge_led_act();
            CHARGE_START();
            set_sys_is_charging(1);
            set_sys_is_charging(0);
            mid_send_to_host("Charging start\n");
            dbg_printf("PWR_12V_SENSE_KEY long down"CRLF);
        }
        if (bmark == PWR_DET_KEY) {
            dbg_printf("PWR_DET_KEY PRESS_LONG_DOWN"CRLF);
            mid_send_to_host("\nshutdown\n");
            cflag |= PWR_DET_KEY;
            set_power_off_led_act();
            set_buzzer_freq(1);
        }
        break;
    case PRESS_CONTINUE_DOWN:
        /*if (bmark == PWR_DET_KEY) {
            dbg_printf("PWR_DET_KEY PRESS_CONTINUE_DOWN"CRLF);
            mid_send_to_host("\nshutdown\n");
            cflag = PWR_DET_KEY;
            set_power_off_led_act();
            BEEP_ON(); get_buzzer_sta() = BIT_BUZZER_ON;
        }*/
        if (bmark == PWR_12V_SENSE_KEY) {
            CLR_LED_ALL_STATUS();
            set_charge_led_act();
            CHARGE_START();
        }
        break;
    default:
        break;
    }
    #undef bmark
}
static void sys_key_init(void)
{
    key_init(&key_data[HEAD_KEY], HEAD_KEY, get_bt_head_key, HIGH);
    key_init(&key_data[TAIL_KEY], TAIL_KEY, get_bt_tail_key, HIGH);
    key_init(&key_data[PWR_DET_KEY], PWR_DET_KEY, get_bt_pwrdet_key, HIGH);
    key_init(&key_data[PWR_12V_SENSE_KEY], PWR_12V_SENSE_KEY, get_pwr_12v_sense_key, LOW);

#if KEY_CALLBACK_ENABLE
    key_attach(&key_data[HEAD_KEY], PRESS_DOWN, key_handle);
    key_attach(&key_data[HEAD_KEY], PRESS_UP, key_handle);

    key_attach(&key_data[TAIL_KEY], PRESS_DOWN, key_handle);
    key_attach(&key_data[TAIL_KEY], PRESS_UP, key_handle);

    /*key_attach(&key_data[PWR_DET_KEY], PRESS_CONTINUE_DOWN, key_handle);*/
    key_attach(&key_data[PWR_DET_KEY], PRESS_LONG_DOWN, key_handle);
    key_attach(&key_data[PWR_DET_KEY], PRESS_DOWN, key_handle);
    key_attach(&key_data[PWR_DET_KEY], PRESS_UP, key_handle);

    key_attach(&key_data[PWR_12V_SENSE_KEY], PRESS_LONG_DOWN, key_handle);
    key_attach(&key_data[PWR_12V_SENSE_KEY], PRESS_CONTINUE_DOWN, key_handle);
    key_attach(&key_data[PWR_12V_SENSE_KEY], PRESS_UP, key_handle);
#endif
    key_start(&key_data[HEAD_KEY]);
    key_start(&key_data[TAIL_KEY]);
    key_start(&key_data[PWR_DET_KEY]);
    key_start(&key_data[PWR_12V_SENSE_KEY]);
}


enum bat_acr_gaurd_t {
    BAT_ACR_GAURD_WARNING = 3000,
    BAT_ACR_GAURD_ERROR = 2000,
    BAT_ACR_GAURD_FATAL = 1000,
    BAT_ACR_GAURD_DEAD = 500,
};


#define BAT_CURR_CHK1                           1000
#define BAT_VOL_CHK1                            10500

#define BAT_CURR_CHK2                           500
#define BAT_VOL_CHK2                            11000

#define BAT_CURR_CHK3                           400
#define BAT_VOL_CHK3                            11100


#define BAT_CURR_CHARGER_CHK                    1000

void sys_init(void)
{
    set_buzzer_off();
    set_buzzer_freq(3);
    sys_key_init();
    dbg_printf("Version:");
    dbg_printf(VERSION);
    dbg_printf(CRLF);
    /*key_test_init();*/
    set_power_on_led_act();
    sched_task_add(task_sys, 0, TASK_LED_FREQ, TASK_PREEMPT_ENABLED);
    sched_task_add(task_rfid, HZ, 1, TASK_PREEMPT_DISABLED);
    sched_task_add(task_dev, HZ, DEV_CYCLES, TASK_PREEMPT_DISABLED);
    /*sched_task_add(task_dev, HZ, HZ * 5, TASK_PREEMPT_DISABLED);*/
#if DEBUG
    sched_task_add(task_test, HZ, HZ, TASK_PREEMPT_DISABLED);
#endif
    if (get_pwr_12v_sense_key() == LOW)
        set_sys_is_charging(0);
    else
        set_sys_is_charging(1);
    set_sys_is_charging(0);
}

void dev_failed_led_act(void)
{
    set_led_freq(5);
    CLR_LED_ALL_STATUS();
    SET_LED_RED_STATUS(BIT_LED_RED_FLASH);
    get_led_0_sta() = get_led_sta();
}
void key_ticks_run(void);
static uint8_t task_sys(void)
{
    static uint8_t led_ticks;
    key_ticks_run();
    handle_port_cmd();
    if (++led_ticks >= get_led_freq()) {
        if (get_led_0_sta())
            handle_led(get_led_0_sta());
        else
            handle_led(get_led_sta());
        led_ticks = 0;
    }
    if (get_buzzer_ticks() < get_buzzer_freq()) {
        ++get_buzzer_ticks();
        BEEP_ON();
    } else {
        if (get_buzzer_sta() == BIT_BUZZER_ON)
            BEEP_ON();
        else
            BEEP_OFF();
    }
    return TASK_RET_NORMAL;
}

__attribute__((unused))
static void check_battery(void)
{
    static uint8_t tickq_w, tickq_e, tickq_em, tickv_em;
    /*if (!get_sys_quantity() || !get_sys_volt())
        return;
    */
    /*1100*/
    if (get_sys_quantity() < BAT_ACR_GAURD_DEAD) {
        tickq_em += 3;
    } else if (get_sys_quantity() < BAT_ACR_GAURD_FATAL) {
        ++tickq_em;
    } else if (get_sys_quantity() < BAT_ACR_GAURD_ERROR) {
        ++tickq_e;
    } else if (get_sys_quantity() < BAT_ACR_GAURD_WARNING) {
        ++tickq_w;
    }

    if (ABS(get_sys_current_total()) > BAT_CURR_CHK1) {
        if (get_sys_volt() < BAT_VOL_CHK1)
            ++tickv_em;
    }
#if 0
    else if (ABS(get_sys_current_total()) > BAT_CURR_CHK2) {
        if (get_sys_volt() < BAT_VOL_CHK2)
            ++tickv_em;
    } else if (ABS(get_sys_current_total()) > BAT_CURR_CHK3) {
        if (get_sys_volt() < BAT_VOL_CHK3)
            ++tickv_em;
    }
#endif
    if (tickq_em > CRITICAL_TRY_COUNT || tickv_em > CRITICAL_V_TRY_COUNT) {
        tickq_em = tickv_em = 0;
        if (get_sys_current_total() > BAT_CURR_CHARGER_CHK) {
            return;
        }
        mid_send_to_host("Fatal error: battery is dead"CRLF);
        set_sys_bat_low_alarm(BAT_LOW_ALARM_FATAL);
        poweroff(5);
    } else if (tickq_e > ERR_TRY_COUNT) {
        tickq_e = 0;
        set_sys_bat_low_alarm(BAT_LOW_ALARM_ERR);
        loading_close();
        mid_send_to_host("Error: lower battery\n");
    }  else if (tickq_w > WARNING_TRY_COUNT) {
        tickq_w = 0;
        set_sys_bat_low_alarm(BAT_LOW_ALARM_WARNING);
        mid_send_to_host("Warning: low battery\n");
    }
}
void loading_close(void)
{
    set_sys_bat_low_alarm(BAT_LOW_ALARM_FATAL);
    motor_stop();
    HEATING0_OFF();
    HEATING1_OFF();
}
__attribute__((unused))
static const  u32 CPUidEncrypt[] = {0xFFFFFFFF, 0x8004000};
__attribute__((unused))
static unsigned char task_dev(void)
{
    dev_chk();
#if USE_CHECK_BAT
    check_battery();
#endif
    motor_check_abnormal();
    return TASK_RET_NORMAL;
}
uint8_t read_rfid(void);
#if DEBUG
/*static uint8_t rfid_ticks;
static uint8_t rfid_flag;*/
static uint8_t task_test(void)
{
    return TASK_RET_NORMAL;
}
#endif
void send_rfid_info(void);
static unsigned char task_rfid(void)
{
#if USE_NEW_PROT
    static uint8_t send = 0;
    static uint16_t ticks;
#endif
    if (!read_rfid()) {
        set_sys_rfid_ack(0);
        set_buzzer_freq(10);
#if USE_NEW_PROT
        send = 3;
#endif
    }
#if USE_NEW_PROT
    if (send) {
        if (++ticks > HZ - 50) {
            ticks = 0;
            --send;
            if (!get_sys_rfid_ack()) {
                send_rfid_info();
            } else {
                send = 0;
            }
        }
    }
#endif
    return TASK_RET_NORMAL;
}
