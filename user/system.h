#ifndef SYSTEM_BB76ZHJ0
#define SYSTEM_BB76ZHJ0
#ifdef __cplusplus
extern "C" { /*}*/
#endif
void sys_init(void);

enum {
    CRITICAL_TRY_COUNT = 10,
    CRITICAL_V_TRY_COUNT = 15,
    ERR_TRY_COUNT = 15,
    WARNING_TRY_COUNT = 20,
};
enum {
    BAT_LOW_ALARM_NORMAL = 0,
    BAT_LOW_ALARM_WARNING = 1,
    BAT_LOW_ALARM_ERR = 2,
    BAT_LOW_ALARM_CRIT = 3,
    BAT_LOW_ALARM_ALERT = 4,
    BAT_LOW_ALARM_FATAL = 5,

};
struct sys_parm_t {
    struct queue protda;
    uint32_t ticks;
    uint8_t r_ack;
    uint8_t p_cnt;
    uint8_t led_sta;
    uint8_t led_sta0;
    uint8_t led_freq;
    uint8_t buzzer_sta;
    uint8_t buzzer_freq;
    uint8_t buzzer_ticks;
    uint8_t is_charging;
    uint8_t devsta;
    uint8_t devfail;
    uint8_t motor_sta;
    uint8_t battery_low_alarm;
    uint8_t buf[3];
    uint16_t current_motor;
    uint16_t volt_total;
    uint32_t quantity;
    int32_t current_total;
};

extern struct sys_parm_t sys_parm;

void poweroff(uint8_t);
void loading_close(void);

#define get_sys_ticks                   sys_parm.ticks
#define get_sys_rfid_ack()              (sys_parm.r_ack)
#define set_sys_rfid_ack(v)             (get_sys_rfid_ack() = (v))
#define get_sys_is_charging()           (sys_parm.is_charging)
#define set_sys_is_charging(_V)         (get_sys_is_charging() = (_V))
#define get_sys_bat_low_alarm()         (sys_parm.battery_low_alarm)
#define set_sys_bat_low_alarm(_V)       (sys_parm.battery_low_alarm = (_V))
#define get_sys_quantity()              (sys_parm.quantity)
#define get_sys_current_total()         (sys_parm.current_total)
#define get_sys_volt()                  (sys_parm.volt_total)

#define set_sys_quantity(_V)            (sys_parm.quantity = (_V))
#define set_sys_current_total(_V)       (sys_parm.current_total = (_V))
#define get_sys_current_motor()         sys_parm.current_motor
#define set_sys_current_motor(_V)       (get_sys_current_motor() = (_V))
#define set_sys_volt(_V)                (sys_parm.volt_total = (_V))

#define get_dev_status()                sys_parm.devsta
#define set_dev_status(_v)              (get_dev_status() |= (_v))
#define get_dev_failed()                sys_parm.devfail
#define set_dev_failed(_V)              (get_dev_failed() |= (_V))
#define prot_cmd                        sys_parm.cmd
#define get_led_0_sta()                 sys_parm.led_sta0
#define get_led_sta()                   sys_parm.led_sta
#define get_led_freq()                  sys_parm.led_freq
#define set_led_freq(V)                 (get_led_freq() = _to_led_freq(V))
#define get_buzzer_ticks()              sys_parm.buzzer_ticks
#define get_buzzer_freq()               sys_parm.buzzer_freq
#define get_buzzer_sta()                sys_parm.buzzer_sta

#define set_buzzer_freq(V)              do { \
                                            BEEP_OFF(); get_buzzer_freq() = _to_buzzer_freq(V); get_buzzer_ticks() = 0; \
                                        } while (0)
#define set_buzzer_on()                 do { \
                                            get_buzzer_ticks() = get_buzzer_freq() + 1; \
                                            get_buzzer_sta() = BIT_BUZZER_ON;  BEEP_ON(); \
                                        } while (0)
#define set_buzzer_off()                do { \
                                            get_buzzer_ticks() = get_buzzer_freq() + 1; \
                                            get_buzzer_sta() = BIT_BUZZER_OFF;  BEEP_OFF(); \
                                        } while (0)
#define get_motor_sta()                 sys_parm.motor_sta
#define set_motor_sta(v)                (sys_parm.motor_sta = (v))

#define CLR_LED_STATUS(v)               (get_led_sta() &= ~(v))
#define CLR_LED_ALL_STATUS()            (CLR_LED_STATUS(BIT_LED_STA_ALL))
#define SET_LED_RED_STATUS(v)           (get_led_sta() = (v | (get_led_sta() & (~(BIT_LED_RED_ON | BIT_LED_RED_FLASH)))))
#define SET_LED_GREEN_STATUS(v)         (get_led_sta() = (v | (get_led_sta() & (~(BIT_LED_GREEN_ON | BIT_LED_GREEN_FLASH)))))
#define SET_LED_BLUE_STATUS(v)          (get_led_sta() = (v | (get_led_sta() & (~(BIT_LED_BLUE_ON| BIT_LED_BLUE_FLASH)))))


#define set_power_on_led_act()          { set_led_freq(1); CLR_LED_ALL_STATUS(); SET_LED_GREEN_STATUS(BIT_LED_GREEN_FLASH); }
#define set_power_off_led_act()         { set_led_freq(1); CLR_LED_ALL_STATUS(); SET_LED_RED_STATUS(BIT_LED_RED_FLASH); }

#define set_charge_led_act()            { set_led_freq(5); SET_LED_GREEN_STATUS(BIT_LED_GREEN_FLASH); }
/*
1HZ  500ms / 10 = 50
2hz  250ms / 10 = 25
3hz  167ms / 10 = 16
4hz  125ms / 10 = 12
5hz  100ms   / 10 = 10
...
8hz  62.5   / 10 = 6
10HZ 50ms  / 10 = 5
*/
#define TASK_LED_FREQ                   10/*ms*/
#define _to_led_freq(V)                 ((500 / TASK_LED_FREQ) / (V))
#define _to_buzzer_freq(V)              ((500 / TASK_LED_FREQ) / (V))

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: SYSTEM_BB76ZHJ0 */
