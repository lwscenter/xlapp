#ifndef COULOMETER_J6UFNPG9
#define COULOMETER_J6UFNPG9
#ifdef __cplusplus
extern "C" { /*}*/
#endif
#define VOTL_GAUD_BLOW                  11560
#define VOTL_GAUD_UPER                  13300
#define VOTLS_SEARCH_ACR                1

#define ds2781_mutex_lock()             do { __disable_irq(); \
        tim3_int_init(1, SystemCoreClock / (HZ * 2) - 1); \
        tick_init(); \
} while (0)
#define ds2781_mutex_unlock()            __enable_irq()

uint8_t ds2781_init(void);

void ds2781_default(void);

void kulunji(void);

void get_coulomb_data(void);

void get_battery_percentage(void);;

uint8_t Battery_Percentage(void);

uint8_t ds2781_self_check(void);

void acr_clear(void);


int32_t get_sys_current_total_(void);

uint16_t get_sys_volt_(void);

uint32_t get_sys_quantity_(void);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: COULOMETER_J6UFNPG9 */


